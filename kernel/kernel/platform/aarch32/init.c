/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <drivers/aarch32/pl050.h>
#include <drivers/aarch32/pl111.h>
#include <drivers/aarch32/pl181.h>
#include <drivers/aarch32/sp804.h>
#include <drivers/aarch32/uart.h>
#include <platform/aarch32/init.h>
#include <platform/aarch32/interrupts.h>

void platform_setup()
{
    interrupts_setup();
}

void platform_drivers_setup()
{
    uart_remap();
    gic_setup();
    sp804_install();
    pl181_install();
    pl111_install();
    pl050_keyboard_install();
    pl050_mouse_install();
}