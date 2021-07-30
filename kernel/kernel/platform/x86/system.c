/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/x86/system.h>
#include <tasking/tasking.h>

void system_disable_interrupts()
{
    THIS_CPU->int_depth_counter++;
    asm volatile("cli");
}

void system_enable_interrupts()
{
    THIS_CPU->int_depth_counter--;
    if (THIS_CPU->int_depth_counter == 0) {
        asm volatile("sti");
    }
}

void system_enable_interrupts_only_counter()
{
    THIS_CPU->int_depth_counter--;
}
