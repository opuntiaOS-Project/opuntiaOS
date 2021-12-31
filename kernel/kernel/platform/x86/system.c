/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/x86/system.h>
#include <tasking/tasking.h>

void system_disable_interrupts()
{
    THIS_CPU->int_depth_counter++;
    system_disable_interrupts_no_counter();
}

void system_enable_interrupts()
{
    THIS_CPU->int_depth_counter--;
    ASSERT(THIS_CPU->int_depth_counter >= 0);
    if (THIS_CPU->int_depth_counter == 0) {
        system_enable_interrupts_no_counter();
    }
}

void system_enable_interrupts_only_counter()
{
    THIS_CPU->int_depth_counter--;
    ASSERT(THIS_CPU->int_depth_counter >= 0);
}
