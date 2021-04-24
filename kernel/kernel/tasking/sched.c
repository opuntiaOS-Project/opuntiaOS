/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/dynamic_array.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmalloc.h>
#include <platform/generic/registers.h>
#include <platform/generic/system.h>
#include <platform/generic/tasking/context.h>
#include <platform/generic/tasking/trapframe.h>
#include <tasking/cpu.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <time/time_manager.h>

// #define SCHED_DEBUG
// #define SCHED_SHOW_STAT

runqueue_t buf1[TOTAL_PRIOS_COUNT];
runqueue_t buf2[TOTAL_PRIOS_COUNT];
static time_t _sched_timeslices[];

// TODO: Made them per-cpu.
static int _buf_read_prio;
static int _enqueued_tasks;
static runqueue_t* _master_buf;
static runqueue_t* _slave_buf;

extern void switch_contexts(context_t** old, context_t* new);
extern void switch_to_context(context_t* new);

/* INIT */
static void _init_cpu(cpu_t* cpu);
/* BUFFERS */
static inline void _sched_swap_buffers();
static inline thread_t* _master_buf_back();
static inline void _sched_save_running_proc();
/* DEBUG */
static void _debug_print_runqueue(runqueue_t* it);

static void _idle_thread()
{
    while (1) {
        system_stop_until_interrupt();
    }
}

static inline time_t _sched_get_timeslice(thread_t* thread)
{
    return _sched_timeslices[thread->process->prio];
}

static void _create_idle_thread(cpu_t* cpu)
{
    proc_t* idle_proc = tasking_create_kernel_thread(_idle_thread, NULL);
    cpu->idle_thread = idle_proc->main_thread;

    // Changing prio.
    sched_dequeue(idle_proc->main_thread);
    idle_proc->prio = IDLE_PRIO;

    sched_enqueue(idle_proc->main_thread);
    _enqueued_tasks -= 1; // Don't count idle thread.
}

static void _init_cpu(cpu_t* cpu)
{
    cpu->current_state = CPU_IN_KERNEL;
    cpu->kstack = kmalloc(VMM_PAGE_SIZE);
    char* sp = cpu->kstack + VMM_PAGE_SIZE;
    sp -= sizeof(*cpu->scheduler);
    cpu->scheduler = (context_t*)sp;
    memset((void*)cpu->scheduler, 0, sizeof(*cpu->scheduler));
    context_set_instruction_pointer(cpu->scheduler, (uint32_t)sched);
    cpu->running_thread = NULL;
#ifdef FPU_ENABLED
    cpu->fpu_for_thread = NULL;
    cpu->fpu_for_pid = 0;
#endif // FPU_ENABLED
    _create_idle_thread(cpu);
}

static inline void _sched_swap_buffers()
{
    runqueue_t* tmp = _master_buf;
    _master_buf = _slave_buf;
    _slave_buf = tmp;
    _buf_read_prio = 0;
}

static inline void _sched_add_to_start_of_runqueue(thread_t* thread)
{
    thread->sched_next = _slave_buf[thread->process->prio].head;
    if (thread->sched_next) {
        thread->sched_next->sched_prev = thread;
    } else {
        _slave_buf[thread->process->prio].tail = thread;
    }
    _slave_buf[thread->process->prio].head = thread;
}

static inline void _sched_add_to_end_of_runqueue(thread_t* thread)
{
    thread->sched_prev = _slave_buf[thread->process->prio].tail;
    if (thread->sched_prev) {
        thread->sched_prev->sched_next = thread;
    } else {
        _slave_buf[thread->process->prio].head = thread;
    }
    _slave_buf[thread->process->prio].tail = thread;
}

void scheduler_init()
{
    _master_buf = buf1;
    _slave_buf = buf2;
    _buf_read_prio = 0;

    for (int i = 0; i < CPU_CNT; i++) {
        _init_cpu(&cpus[i]);
    }
}

extern thread_t thread_storage[512];
extern int threads_cnt;
void sched_unblock_threads()
{
    thread_t* thread;
    for (int i = 0; i < threads_cnt; i++) {
        thread = &thread_storage[i];
        if (thread->status == THREAD_BLOCKED && thread->blocker.reason != BLOCKER_INVALID) {
            if (thread->blocker.should_unblock && thread->blocker.should_unblock(thread)) {
                thread->status = THREAD_RUNNING;
                thread->blocker.reason = BLOCKER_INVALID;
                sched_enqueue(thread);
            }
        }
    }
}

void resched_dont_save_context()
{
    if (RUNNING_THREAD && RUNNING_THREAD->status == THREAD_RUNNING) {
        RUNNING_THREAD->stat_total_running_ticks += timeman_ticks_since_boot() - RUNNING_THREAD->start_time_in_ticks;
        _sched_add_to_end_of_runqueue(RUNNING_THREAD);
    }
    switch_to_context(THIS_CPU->scheduler);
}

void resched()
{
    if (RUNNING_THREAD) {
        RUNNING_THREAD->stat_total_running_ticks += timeman_ticks_since_boot() - RUNNING_THREAD->start_time_in_ticks;
        if (RUNNING_THREAD->status == THREAD_RUNNING) {
            _sched_add_to_end_of_runqueue(RUNNING_THREAD);
        }
        switch_contexts(&RUNNING_THREAD->context, THIS_CPU->scheduler);
    } else {
        switch_to_context(THIS_CPU->scheduler);
    }
}

void sched_enqueue(thread_t* thread)
{
    thread->status = THREAD_RUNNING;
#ifdef SCHED_DEBUG
    log("enqueue task %d\n", thread->tid);
#endif
    if (thread->process->prio > MIN_PRIO) {
        thread->process->prio = MIN_PRIO;
    }

    _sched_add_to_start_of_runqueue(thread);
    _enqueued_tasks++;
}

void sched_dequeue(thread_t* thread)
{
#ifdef SCHED_DEBUG
    log("dequeue task %d\n", thread->tid);
#endif
    if (_slave_buf[thread->process->prio].tail == thread) {
        _slave_buf[thread->process->prio].tail = thread->sched_prev;
    }

    if (_slave_buf[thread->process->prio].head == thread) {
        _slave_buf[thread->process->prio].head = thread->sched_next;
    }

    if (_master_buf[thread->process->prio].tail == thread) {
        _master_buf[thread->process->prio].tail = thread->sched_prev;
    }

    if (_master_buf[thread->process->prio].head == thread) {
        _master_buf[thread->process->prio].head = thread->sched_next;
    }

    if (thread->sched_prev) {
        thread->sched_prev->sched_next = thread->sched_next;
    }

    if (thread->sched_next) {
        thread->sched_next->sched_prev = thread->sched_prev;
    }

    thread->sched_next = thread->sched_prev = NULL;
    _enqueued_tasks--;
}

void sched()
{
    for (;;) {
        while (!_master_buf[_buf_read_prio].head) {
            _buf_read_prio++;
            if (_buf_read_prio >= IDLE_PRIO) {
                tasking_kill_dying();
                sched_unblock_threads();
                _sched_swap_buffers();
            }
        }

        thread_t* thread = _master_buf[_buf_read_prio].head;
        _master_buf[_buf_read_prio].head = thread->sched_next;
        if (_master_buf[_buf_read_prio].tail == thread) {
            _master_buf[_buf_read_prio].tail = NULL;
        }
        thread->sched_next = thread->sched_prev = NULL;
#ifdef SCHED_DEBUG
        log("next to run %d %x %x\n", thread->tid, get_instruction_pointer(thread->tf), thread->tf);
#endif
#ifdef SCHED_SHOW_STAT
        log("[STAT] procs in buffer: %d", _debug_count_of_proc_in_buf(_master_buf));
#endif
        ASSERT(thread->status == THREAD_RUNNING);
        thread->start_time_in_ticks = timeman_ticks_since_boot();
        thread->ticks_until_preemption = _sched_get_timeslice(thread);
        switchuvm(thread);
        switch_contexts(&(THIS_CPU->scheduler), thread->context);
    }
}

static void _debug_print_runqueue(runqueue_t* it)
{
    for (int i = 0; i < PROC_PRIOS_COUNT; i++) {
        log(" Prio %d", i);
        thread_t* tmp = it[i].head;
        while (tmp) {
            log("   %d ->", tmp->tid);
            tmp = tmp->sched_next;
        }
    }
}

static time_t _sched_timeslices[] = {
    [0] = 5,
    [1] = 5,
    [2] = 4,
    [3] = 4,
    [4] = 4,
    [5] = 3,
    [6] = 3,
    [7] = 3,
    [8] = 3,
    [9] = 2,
    [10] = 2,
    [11] = 1,
};