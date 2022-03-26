/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/dynamic_array.h>
#include <libkern/atomic.h>
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

static time_t _sched_timeslices[];
static int _enqueued_tasks;
static size_t _active_cpus;

extern void switch_contexts(context_t** old, context_t* new);
extern void switch_to_context(context_t* new);

/* INIT */
static void _init_cpu(cpu_t* cpu);
/* BUFFERS */
static inline void _sched_swap_buffers();
static inline thread_t* _master_buf_back();
static inline void _sched_save_running_proc();
static inline void _sched_enqueue_impl(sched_data_t* sched, thread_t* thread);
/* DEBUG */
static void _debug_print_runqueue(runqueue_t* it);

/**
 * Idle thread runs when no workload is available on a cpu.
 *
 * Idle thread should be available to run all the time. In order to
 * to avoid blocking, allocations are forbidden when an idle thread
 * is running, thus interrupts should guarantee no allocation
 * during handling all of them.
 */
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
    idle_proc->prio = IDLE_PRIO;
    _sched_enqueue_impl(&cpu->sched, idle_proc->main_thread);
}

size_t active_cpu_count()
{
    return atomic_load(&_active_cpus);
}

static inline void _add_cpu_count()
{
    atomic_add(&_active_cpus, 1);
}

static void _init_cpu(cpu_t* cpu)
{
    cpu->current_state = CPU_IN_KERNEL;
    cpu->data_access_type = DATA_ACCESS_REGULAR;

    cpu->sched_stack_zone = kmemzone_new(VMM_PAGE_SIZE);
    vmm_alloc_page(cpu->sched_stack_zone.start, MMU_FLAG_PERM_READ | MMU_FLAG_PERM_WRITE);
    uint8_t* sp = cpu->sched_stack_zone.ptr + VMM_PAGE_SIZE;
    sp -= sizeof(*cpu->sched_context);
    cpu->sched_context = (context_t*)sp;
    memset((void*)cpu->sched_context, 0, sizeof(*cpu->sched_context));
    context_set_instruction_pointer(cpu->sched_context, (uintptr_t)sched);
    cpu->running_thread = NULL;

    cpu->sched.master_buf = kmalloc(sizeof(runqueue_t) * TOTAL_PRIOS_COUNT);
    cpu->sched.slave_buf = kmalloc(sizeof(runqueue_t) * TOTAL_PRIOS_COUNT);
    memset(cpu->sched.master_buf, 0, sizeof(runqueue_t) * TOTAL_PRIOS_COUNT);
    memset(cpu->sched.slave_buf, 0, sizeof(runqueue_t) * TOTAL_PRIOS_COUNT);
    cpu->sched.next_read_prio = 0;
    cpu->sched.enqueued_tasks = 0;

#ifdef FPU_ENABLED
    cpu->fpu_for_thread = NULL;
    cpu->fpu_for_pid = 0;
#endif // FPU_ENABLED
    _create_idle_thread(cpu);
    _add_cpu_count();
}

static inline void _sched_swap_buffers(sched_data_t* sched)
{
    runqueue_t* tmp = sched->master_buf;
    sched->master_buf = sched->slave_buf;
    sched->slave_buf = tmp;
    sched->next_read_prio = 0;
}

static inline void _sched_add_to_start_of_runqueue(sched_data_t* sched, thread_t* thread)
{
    thread->sched_next = sched->slave_buf[thread->process->prio].head;
    if (thread->sched_next) {
        thread->sched_next->sched_prev = thread;
    } else {
        sched->slave_buf[thread->process->prio].tail = thread;
    }
    sched->slave_buf[thread->process->prio].head = thread;
}

static inline void _sched_add_to_end_of_runqueue(sched_data_t* sched, thread_t* thread)
{
    thread->sched_prev = sched->slave_buf[thread->process->prio].tail;
    if (thread->sched_prev) {
        thread->sched_prev->sched_next = thread;
    } else {
        sched->slave_buf[thread->process->prio].head = thread;
    }
    sched->slave_buf[thread->process->prio].tail = thread;
}

static inline void _sched_enqueue_impl(sched_data_t* sched, thread_t* thread)
{
    _sched_add_to_start_of_runqueue(sched, thread);
    sched->enqueued_tasks++;
}

void _sched_dequeue_impl(sched_data_t* sched, thread_t* thread)
{
    if (sched->slave_buf[thread->process->prio].tail == thread) {
        sched->slave_buf[thread->process->prio].tail = thread->sched_prev;
    }

    if (sched->slave_buf[thread->process->prio].head == thread) {
        sched->slave_buf[thread->process->prio].head = thread->sched_next;
    }

    if (sched->master_buf[thread->process->prio].tail == thread) {
        sched->master_buf[thread->process->prio].tail = thread->sched_prev;
    }

    if (sched->master_buf[thread->process->prio].head == thread) {
        sched->master_buf[thread->process->prio].head = thread->sched_next;
    }

    if (thread->sched_prev) {
        thread->sched_prev->sched_next = thread->sched_next;
    }

    if (thread->sched_next) {
        thread->sched_next->sched_prev = thread->sched_prev;
    }

    thread->sched_next = thread->sched_prev = NULL;
    sched->enqueued_tasks--;
}

int _sched_find_cpu_with_less_load()
{
    int mx = cpus[0].sched.enqueued_tasks;
    int id = 0;
    for (int i = 1; i < active_cpu_count(); i++) {
        if (mx > cpus[i].sched.enqueued_tasks) {
            mx = cpus[i].sched.enqueued_tasks;
            id = i;
        }
    }
    return id;
}

void scheduler_init()
{
}

void schedule_activate_cpu()
{
    int id = system_cpu_id();
    ASSERT(id < MAX_CPU_CNT);
    _init_cpu(&cpus[id]);
    cpus[id].id = id;
}

extern thread_list_t thread_list;
void sched_unbspinlock_threads()
{
    thread_t* thread;

    thread_list_node_t* __thread_list_node = thread_list.head;
    while (__thread_list_node) {
        for (int i = 0; i < THREADS_PER_NODE; i++) {
            thread = &__thread_list_node->thread_storage[i];
            if (thread->status == THREAD_STATUS_BLOCKED && thread->blocker.reason != BLOCKER_INVALID) {
                if (thread->blocker.should_unblock && thread->blocker.should_unblock(thread)) {
                    thread->status = THREAD_STATUS_RUNNING;
                    thread->blocker.reason = BLOCKER_INVALID;
                    sched_enqueue(thread);
                }
            }
        }
        __thread_list_node = __thread_list_node->next;
    }
}

void resched_dont_save_context()
{
    // Add the thread back to runqueue only if thread is still running.
    if (RUNNING_THREAD && RUNNING_THREAD->status == THREAD_STATUS_RUNNING) {
        RUNNING_THREAD->stat_total_running_ticks += timeman_ticks_since_boot() - RUNNING_THREAD->start_time_in_ticks;
        _sched_add_to_end_of_runqueue(&cpus[RUNNING_THREAD->last_cpu].sched, RUNNING_THREAD);
    }
    switch_to_context(THIS_CPU->sched_context);
}

void resched()
{
    if (likely(RUNNING_THREAD)) {
        RUNNING_THREAD->stat_total_running_ticks += timeman_ticks_since_boot() - RUNNING_THREAD->start_time_in_ticks;
        // Add the thread back to runqueue only if thread is still running.
        if (RUNNING_THREAD->status == THREAD_STATUS_RUNNING) {
            _sched_add_to_end_of_runqueue(&cpus[RUNNING_THREAD->last_cpu].sched, RUNNING_THREAD);
        }
        switch_contexts(&RUNNING_THREAD->context, THIS_CPU->sched_context);
    } else {
        switch_to_context(THIS_CPU->sched_context);
    }
}

void sched_enqueue(thread_t* thread)
{
    thread->status = THREAD_STATUS_RUNNING;
    if (thread->process->prio > MIN_PRIO) {
        thread->process->prio = MIN_PRIO;
    }

    if (thread->last_cpu != LAST_CPU_NOT_SET) {
        _sched_enqueue_impl(&cpus[thread->last_cpu].sched, thread);
    } else {
        int cpu = _sched_find_cpu_with_less_load();
        _sched_enqueue_impl(&cpus[cpu].sched, thread);
        thread->last_cpu = cpu;
    }

#ifdef SCHED_DEBUG
    log("enqueue task %d to cpu %d", thread->tid, thread->last_cpu);
#endif
    _enqueued_tasks++;
}

void sched_dequeue(thread_t* thread)
{
#ifdef SCHED_DEBUG
    log("dequeue task %d", thread->tid);
#endif
    if (likely(thread->last_cpu != LAST_CPU_NOT_SET)) {
        _sched_dequeue_impl(&cpus[thread->last_cpu].sched, thread);
    } else {
        log("dequeue error task %d", thread->tid);
    }
}

static void switch_to_thread(thread_t* thread)
{
    if (thread->pending_signals_mask) {
        signal_dispatch_pending(thread);
        if (thread->status != THREAD_STATUS_RUNNING) {
            // Signals could terminate/block/stop the thread, so check if it's still runnable.
            return;
        }
    }

    thread->last_cpu = THIS_CPU->id;
    thread->start_time_in_ticks = timeman_ticks_since_boot();
    thread->ticks_until_preemption = _sched_get_timeslice(thread);
    switchuvm(thread);
    switch_contexts(&(THIS_CPU->sched_context), thread->context);
}

void sched()
{
    for (;;) {
        sched_data_t* sched = &THIS_CPU->sched;
        while (!sched->master_buf[sched->next_read_prio].head) {
            sched->next_read_prio++;
            if (sched->next_read_prio >= TOTAL_PRIOS_COUNT) {
                if (THIS_CPU->id == 0) {
                    tasking_kill_dying();
                    sched_unbspinlock_threads();
                }
                _sched_swap_buffers(sched);
            }
        }

        thread_t* thread = sched->master_buf[sched->next_read_prio].head;
        sched->master_buf[sched->next_read_prio].head = thread->sched_next;
        if (sched->master_buf[sched->next_read_prio].tail == thread) {
            sched->master_buf[sched->next_read_prio].tail = NULL;
        }
        if (thread->sched_next) {
            thread->sched_next->sched_prev = NULL;
        }
        thread->sched_next = thread->sched_prev = NULL;
#ifdef SCHED_DEBUG
        log("next to run %d %x %x [cpu %d]", thread->tid, thread->process->prio, thread->tf, THIS_CPU->id);
#endif
#ifdef SCHED_SHOW_STAT
        _debug_print_runqueue(sched->master_buf);
#endif
        ASSERT(thread->status == THREAD_STATUS_RUNNING);
        switch_to_thread(thread);
    }
}

static void _debug_print_runqueue(runqueue_t* it)
{
    for (int i = 0; i < TOTAL_PRIOS_COUNT; i++) {
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
    [12] = 1,
};