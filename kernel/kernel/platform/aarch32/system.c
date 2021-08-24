/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* FIXME: Only for one cpu now */

#include <libkern/libkern.h>
#include <platform/aarch32/system.h>
#include <tasking/tasking.h>

void system_disable_interrupts()
{
    THIS_CPU->int_depth_counter++;
    asm volatile("cpsid i");
}

void system_enable_interrupts()
{
    THIS_CPU->int_depth_counter--;
    ASSERT(THIS_CPU->int_depth_counter >= 0);
    if (THIS_CPU->int_depth_counter == 0) {
        asm volatile("cpsie i");
    }
}

void system_enable_interrupts_only_counter()
{
    THIS_CPU->int_depth_counter--;
    ASSERT(THIS_CPU->int_depth_counter >= 0);
}
