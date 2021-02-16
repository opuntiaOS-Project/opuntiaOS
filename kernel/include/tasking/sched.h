/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef __oneOS__X86__TASKING__SCHED_H
#define __oneOS__X86__TASKING__SCHED_H

#include <mem/vmm/vmm.h>
#include <tasking/tasking.h>
#include <types.h>

#define MAX_PRIO 0
#define MIN_PRIO 10
#define PRIOS_COUNT (MIN_PRIO - MAX_PRIO + 1)
#define DEFAULT_PRIO 6
#define SCHED_INT 10

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

#endif // __oneOS__X86__TASKING__SCHED_H