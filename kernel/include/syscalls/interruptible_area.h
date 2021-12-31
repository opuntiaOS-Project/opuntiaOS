/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_SYSCALLS_INTERRUPTIBLE_AREA_H
#define _KERNEL_SYSCALLS_INTERRUPTIBLE_AREA_H

#include <libkern/libkern.h>
#include <platform/generic/system.h>
#include <tasking/cpu.h>

/**
 * By design the whole kernel is not interruptible, but some parts inside kernel
 * could be interrupted without any bad consequences and at the same time these
 * parts take a lot of cpu time, like copying data from/to user. We might want
 * to mark them as interruptible.
 */

static ALWAYS_INLINE int interruptible_area_start()
{
    int res = THIS_CPU->int_depth_counter;
    THIS_CPU->int_depth_counter = 0;
    system_enable_interrupts_no_counter();
    return res;
}

static ALWAYS_INLINE int interruptible_area_end(int old)
{
    system_disable_interrupts_no_counter();
    THIS_CPU->int_depth_counter = old;
    return 0;
}

#define INTERRUPTIBLE_AREA for (int __i = 0, __j = interruptible_area_start(); __i < 1; interruptible_area_end(__j), __i++)

#endif // _KERNEL_SYSCALLS_INTERRUPTIBLE_AREA_H
