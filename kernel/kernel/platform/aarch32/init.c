/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/fpuv4.h>
#include <drivers/aarch32/pl031.h>
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
    fpuv4_install();
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
    pl031_install();
}