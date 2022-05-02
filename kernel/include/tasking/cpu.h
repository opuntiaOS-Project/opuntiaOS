/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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

#define RUNNING_THREAD (THIS_CPU->running_thread)

static inline cpu_state_t cpu_enter_kernel_space()
{
    cpu_state_t prev = THIS_CPU->current_state;
    THIS_CPU->current_state = CPU_IN_KERNEL;
    return prev;
}

static inline cpu_state_t cpu_get_state()
{
    return THIS_CPU->current_state;
}

static inline void cpu_set_state(cpu_state_t state)
{
    THIS_CPU->current_state = state;
}

static inline void cpu_enter_user_space()
{
    THIS_CPU->current_state = CPU_IN_USERLAND;
}

static inline void cpu_tick()
{
    // if (!THIS_CPU->running_thread) {
    //     THIS_CPU->stat_system_and_idle_ticks++;
    // } else if (THIS_CPU->running_thread->process->is_kthread) {
    //     THIS_CPU->stat_system_and_idle_ticks++;
    // } else {
    //     THIS_CPU->stat_user_ticks++;
    // }
    ASSERT(false);
}

#endif // _KERNEL_TASKING_CPU_H
