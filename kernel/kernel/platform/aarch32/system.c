/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* FIXME: Only for one cpu now */

#include <platform/aarch32/system.h>
#include <libkern/libkern.h>

static int depth_counter = 0;

void system_disable_interrupts()
{
    depth_counter++;
    asm volatile("cpsid i");
}

void system_enable_interrupts()
{
    depth_counter--;
    ASSERT(depth_counter >= 0);
    if (depth_counter == 0) {
        asm volatile("cpsie i");
    }
}

void system_enable_interrupts_only_counter()
{
    depth_counter--;
    ASSERT(depth_counter >= 0);
}
