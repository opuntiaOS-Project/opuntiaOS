/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_CPU_H
#define _KERNEL_TASKING_CPU_H

#include <drivers/generic/fpu.h>
#include <tasking/bits/cpu.h>

static inline void cpu_enter_kernel_space()
{
    THIS_CPU->current_state = CPU_IN_KERNEL;
}

static inline void cpu_leave_kernel_space()
{
    THIS_CPU->current_state = CPU_IN_USERLAND;
}

#endif // _KERNEL_TASKING_CPU_H
