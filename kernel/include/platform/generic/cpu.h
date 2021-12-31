/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_BITS_CPU_H
#define _KERNEL_TASKING_BITS_CPU_H

#include <drivers/generic/fpu.h>
#include <libkern/types.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <platform/generic/tasking/context.h>
#include <tasking/bits/sched.h>

#define CPU_CNT 4
#define THIS_CPU (&cpus[system_cpu_id()])
#define FPU_ENABLED

struct thread;
typedef int cpu_state_t;
enum CPU_STATE {
    CPU_IN_KERNEL,
    CPU_IN_USERLAND,
};

typedef struct {
    int id;
    int int_depth_counter;

    pdirectory_t* pdir;
    kmemzone_t sched_stack_zone;
    context_t* sched_context; // context of sched's registers
    struct thread* running_thread;
    cpu_state_t current_state;
    struct thread* idle_thread;

    sched_data_t sched;

    /* Stat */
    time_t stat_ticks_since_boot;
    time_t stat_system_and_idle_ticks;
    time_t stat_user_ticks;

#ifdef FPU_ENABLED
    // Information about current state of fpu.
    struct thread* fpu_for_thread;
    pid_t fpu_for_pid;
#endif // FPU_ENABLED
} cpu_t;

extern cpu_t cpus[CPU_CNT];

#endif // _KERNEL_TASKING_BITS_CPU_H