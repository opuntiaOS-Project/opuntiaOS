/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* FIXME: Only for one cpu now */

#include <platform/x86/system.h>

static int depth_counter = 0;

void system_disable_interrupts()
{
    depth_counter++;
    asm volatile("cli");
}

void system_enable_interrupts()
{
    depth_counter--;
    if (depth_counter == 0) {
        asm volatile("sti");
    }
}

void system_enable_interrupts_only_counter()
{
    depth_counter--;
}
