/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_BITS_SCHED_H
#define _KERNEL_TASKING_BITS_SCHED_H

#define MAX_PRIO 0
#define MIN_PRIO 11
#define IDLE_PRIO (MIN_PRIO + 1)
#define PROC_PRIOS_COUNT (MIN_PRIO - MAX_PRIO + 1)
#define TOTAL_PRIOS_COUNT (IDLE_PRIO - MAX_PRIO + 1)
#define DEFAULT_PRIO 6
#define SCHED_INT 10
#define LAST_CPU_NOT_SET 0xffff

struct thread;

struct runqueue {
    struct thread* head;
    struct thread* tail;
};
typedef struct runqueue runqueue_t;

struct sched_data {
    int next_read_prio;
    runqueue_t* master_buf;
    runqueue_t* slave_buf;
    int enqueued_tasks;
};
typedef struct sched_data sched_data_t;

#endif // _KERNEL_TASKING_BITS_SCHED_H