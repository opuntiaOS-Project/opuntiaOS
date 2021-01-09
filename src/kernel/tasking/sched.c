/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <algo/dynamic_array.h>
#include <log.h>
#include <mem/kmalloc.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <utils/kassert.h>
#include <platform/x86/registers.h>

// #define SCHED_DEBUG
// #define SCHED_SHOW_STAT

runqueue_t buf1[PRIOS_COUNT];
runqueue_t buf2[PRIOS_COUNT];

static int _buf_read_prio;
static runqueue_t* _master_buf;
static runqueue_t* _slave_buf;

extern void switch_contexts(context_t** old, context_t* new);
extern void switch_to_context(context_t* new);

/* INIT */
static void _init_cpus(cpu_t* cpu);
/* BUFFERS */
static inline void _sched_swap_buffers();
static inline thread_t* _master_buf_back();
static inline void _sched_save_running_proc();
/* DEBUG */
static void _debug_print_runqueue(runqueue_t* it);

static void _init_cpus(cpu_t* cpu)
{
    cpu->kstack = kmalloc(VMM_PAGE_SIZE);
    char* sp = cpu->kstack + VMM_PAGE_SIZE;
    sp -= sizeof(*cpu->scheduler);
    cpu->scheduler = (context_t*)sp;
    memset((void*)cpu->scheduler, 0, sizeof(*cpu->scheduler));
    cpu->scheduler->eip = (uint32_t)sched;
    cpu->running_thread = 0;
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
        _init_cpus(&cpus[i]);
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
            if (thread->blocker.should_unblock(thread)) {
                thread->status = THREAD_RUNNING;
                thread->blocker.reason = BLOCKER_INVALID;
                sched_enqueue(thread);
            }
        }
    }
}

void resched_dont_save_context()
{
    if (RUNNIG_THREAD && RUNNIG_THREAD->status == THREAD_RUNNING) {
        _sched_add_to_end_of_runqueue(RUNNIG_THREAD);
    }
    switch_to_context(THIS_CPU->scheduler);
}

void resched()
{
    if (RUNNIG_THREAD) {
        if (RUNNIG_THREAD->status == THREAD_RUNNING) {
            _sched_add_to_end_of_runqueue(RUNNIG_THREAD);
        }
        switch_contexts(&RUNNIG_THREAD->context, THIS_CPU->scheduler);
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

    thread->sched_next = thread->sched_prev = 0;
}

void sched()
{
    for (;;) {
        while (!_master_buf[_buf_read_prio].head) {
            if (_buf_read_prio >= MIN_PRIO) {
                tasking_kill_dying();
                sched_unblock_threads(); 
                _sched_swap_buffers();
            } else {
                _buf_read_prio++;
            }
        }

        thread_t* thread = _master_buf[_buf_read_prio].head;
        _master_buf[_buf_read_prio].head = thread->sched_next;
        if (_master_buf[_buf_read_prio].tail == thread) {
            _master_buf[_buf_read_prio].tail = 0;
        }
        thread->sched_next = thread->sched_prev = 0;
#ifdef SCHED_DEBUG
        log("next to run %d %x\n", thread->tid, thread->tf->eip);
#endif
#ifdef SCHED_SHOW_STAT
        log("[STAT] procs in buffer: %d", _debug_count_of_proc_in_buf(_master_buf));
#endif
        ASSERT(thread->status == THREAD_RUNNING);
        switchuvm(thread);
        switch_contexts(&THIS_CPU->scheduler, thread->context);
    }
}

static void _debug_print_runqueue(runqueue_t* it)
{
    for (int i = 0; i < PRIOS_COUNT; i++) {
        log(" Prio %d", i);
        thread_t* tmp = it[i].head;
        while (tmp) {
            log("   %d ->", tmp->tid);
            tmp = tmp->sched_next;
        }
    }
}