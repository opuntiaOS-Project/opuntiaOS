/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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

/**
 * platform_init_boot_cpu initializes bare minimum to setup VM.
 */
void platform_init_boot_cpu()
{
    interrupts_setup();
}

void platform_setup_boot_cpu()
{
    fpuv4_install();
    gic_setup();
}

void platform_setup_secondary_cpu()
{
    interrupts_setup_secondary_cpu();
    fpuv4_install();
    gic_setup_secondary_cpu();
}

void platform_drivers_setup()
{
    uart_remap();
    sp804_install();
    pl181_install();
    pl111_install();
    pl050_keyboard_install();
    pl050_mouse_install();
    pl031_install();
}