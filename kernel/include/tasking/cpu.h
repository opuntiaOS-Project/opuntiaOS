/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_CPU_H
#define _KERNEL_TASKING_CPU_H

#include <drivers/generic/fpu.h>
#include <platform/generic/cpu.h>
#include <tasking/bits/sched.h>
#include <tasking/proc.h>
#include <tasking/thread.h>

#define RUNNING_THREAD (THIS_CPU->running_thread)

static inline void cpu_enter_kernel_space()
{
    THIS_CPU->current_state = CPU_IN_KERNEL;
}

static inline void cpu_leave_kernel_space()
{
    THIS_CPU->current_state = CPU_IN_USERLAND;
}

static inline void cpu_tick()
{
    if (THIS_CPU->running_thread->process->is_kthread) {
        THIS_CPU->stat_system_and_idle_ticks++;
    } else {
        THIS_CPU->stat_user_ticks++;
    }
}

#endif // _KERNEL_TASKING_CPU_H
