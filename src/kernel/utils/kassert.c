/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <drivers/x86/display.h>

void kpanic(char* err_msg)
{
    clean_screen();
    kprintf("*****\n");
    kprintf("Kernel Panic\n");
    kprintf("*****\n");
    kprintf(err_msg);
    kprintf("\n*****");
    asm volatile("cli\n");
    asm volatile("hlt\n");
}