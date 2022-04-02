/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_SCHED_H
#define _KERNEL_TASKING_SCHED_H

#include <libkern/types.h>
#include <mem/vmm.h>
#include <tasking/bits/sched.h>
#include <tasking/tasking.h>

void scheduler_init();
void schedule_activate_cpu();
void resched_dont_save_context();
void resched();
void sched();
void sched_enqueue(thread_t* thread);
void sched_dequeue(thread_t* thread);
size_t active_cpu_count();

static inline void sched_tick()
{
    if (!RUNNING_THREAD) {
        return;
    }

    RUNNING_THREAD->ticks_until_preemption--;
    if (!RUNNING_THREAD->ticks_until_preemption) {
        log("   will resched");
        resched();
    }
}

#endif // _KERNEL_TASKING_SCHED_H