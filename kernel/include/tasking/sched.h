/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_SCHED_H
#define _KERNEL_TASKING_SCHED_H

#include <libkern/types.h>
#include <mem/vmm/vmm.h>
#include <tasking/bits/sched.h>
#include <tasking/tasking.h>

struct runqueue {
    thread_t* head;
    thread_t* tail;
};
typedef struct runqueue runqueue_t;

void scheduler_init();
void resched_dont_save_context();
void resched();
void sched();
void sched_enqueue(thread_t* thread);
void sched_dequeue(thread_t* thread);

static inline void sched_tick()
{
    if (RUNNING_THREAD) {
        RUNNING_THREAD->ticks_until_preemption--;
        if (!RUNNING_THREAD->ticks_until_preemption) {
            resched();
        }
    }
}

#endif // _KERNEL_TASKING_SCHED_H