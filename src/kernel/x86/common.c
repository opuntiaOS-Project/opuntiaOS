/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

/* FIXME: Only for one cpu now */

#include <x86/common.h>

static int depth_counter = 0;

void cli()
{
    depth_counter++;
    asm volatile("cli");
}

void sti()
{
    depth_counter--;
    if (depth_counter == 0) {
        asm volatile("sti");
    }
}