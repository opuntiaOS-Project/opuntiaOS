/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

/* FIXME: Only for one cpu now */

#include <platform/aarch32/system.h>
#include <utils.h>

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
