/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_BITS_CPU_H
#define _KERNEL_TASKING_BITS_CPU_H

#include <drivers/generic/fpu.h>
#include <libkern/types.h>
#include <platform/generic/tasking/context.h>

#define CPU_CNT 1
#define THIS_CPU (&cpus[0])
#define RUNNIG_THREAD (THIS_CPU->running_thread)
#define FPU_ENABLED

struct thread;
typedef int cpu_state_t;
enum CPU_STATE {
    CPU_IN_KERNEL,
    CPU_IN_USERLAND,
};

typedef struct {
    char* kstack;
    context_t* scheduler; // context of sched's registers
    struct thread* running_thread;
    cpu_state_t current_state;
#ifdef FPU_ENABLED
    // Information about current state of fpu.
    struct thread* fpu_for_thread;
    pid_t fpu_for_pid;
#endif // FPU_ENABLED
} cpu_t;

extern cpu_t cpus[CPU_CNT];

#endif // _KERNEL_TASKING_BITS_CPU_H