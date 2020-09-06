/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <algo/dynamic_array.h>
#include <mem/kmalloc.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>
#include <utils/kassert.h>

// #define SCHED_DEBUG
// #define SCHED_SHOW_STAT

dynamic_array_t buf1[PRIOS_COUNT];
dynamic_array_t buf2[PRIOS_COUNT];

static int _buf_read_prio;
static dynamic_array_t* _master_buf;
static dynamic_array_t* _slave_buf;

extern void switch_contexts(context_t** old, context_t* new);
extern void switch_to_context(context_t* new);

/* INIT */
static void _init_cpus(cpu_t* cpu);
static void _sched_init_buf(dynamic_array_t* buf);
/* BUFFERS */
static inline void _sched_swap_buffers();
static inline thread_t* _master_buf_back();
static inline void _sched_save_running_proc();
/* DEBUG */
static uint32_t _debug_count_of_proc_in_buf(dynamic_array_t* buf);

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

static void _sched_init_buf(dynamic_array_t* buf)
{
    for (int i = 0; i < PRIOS_COUNT; i++) {
        dynamic_array_init_of_size(&buf[i], sizeof(thread_t*), 8);
    }
}

static inline void _sched_swap_buffers()
{
    dynamic_array_t* tmp = _master_buf;
    _master_buf = _slave_buf;
    _slave_buf = tmp;
    _buf_read_prio = 0;
}

static inline thread_t* _master_buf_back()
{
    return *((thread_t**)(dynamic_array_get(&_master_buf[_buf_read_prio], _master_buf[_buf_read_prio].size - 1)));
}

static inline void _sched_save_running_proc()
{
    dynamic_array_push(&_slave_buf[RUNNIG_THREAD->process->prio], &RUNNIG_THREAD);
}

void scheduler_init()
{
    _sched_init_buf(buf1);
    _sched_init_buf(buf2);
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
    // TODO: Run each thread in proc
    thread_t* thread;
    for (int i = 0; i < threads_cnt; i++) {
        thread = &thread_storage[i];
        if (thread->status == THREAD_BLOCKED && thread->blocker.reason != BLOCKER_INVALID) {
            if (thread->blocker.should_unblock(thread)) {
                thread->status = THREAD_RUNNING;
                sched_enqueue(thread);
            }
        }
    }
}


void resched()
{
    tasking_kill_dying();
    sched_unblock_threads();
    if (RUNNIG_THREAD) {
        _sched_save_running_proc();
        switch_contexts(&RUNNIG_THREAD->context, THIS_CPU->scheduler);
    } else {
        switch_to_context(THIS_CPU->scheduler);
    }
}

void sched_enqueue(thread_t* thread)
{
    thread->status = THREAD_RUNNING;
#ifdef SCHED_DEBUG
    kprintf("enqueue %d\n", thread->tid);
#endif
    if (thread->process->prio > MIN_PRIO) {
        thread->process->prio = MIN_PRIO;
    }

    dynamic_array_push(&_slave_buf[thread->process->prio], &thread);
}

void sched_dequeue(thread_t* p)
{

}

void sched()
{
    for (;;) {
        while (_master_buf[_buf_read_prio].size == 0) {
            if (_buf_read_prio > MIN_PRIO) {
                _sched_swap_buffers();
            } else {
                _buf_read_prio++;
            }
        }


        thread_t* thread = _master_buf_back();
        dynamic_array_pop(&_master_buf[_buf_read_prio]);
#ifdef SCHED_SHOW_STAT
        kprintf("%d", _debug_count_of_proc_in_buf(_master_buf));
#endif
        if (thread->status == THREAD_RUNNING) {
            // kprintf("run %d\n", thread->tid);
            switchuvm(thread);
            switch_contexts(&THIS_CPU->scheduler, thread->context);
        }
    }
}

static uint32_t _debug_count_of_proc_in_buf(dynamic_array_t* buf)
{
    uint32_t res = 0;
    for (int i = 0; i < PRIOS_COUNT; i++) {
        res += buf[i].size;
    }
    return res;
}