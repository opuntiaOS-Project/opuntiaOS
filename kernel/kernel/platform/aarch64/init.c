/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/irq/arm/gicv2.h>
#include <platform/aarch64/fpu/fpu.h>
#include <platform/aarch64/init.h>
#include <platform/aarch64/interrupts.h>

int core_setup()
{
    return 0;
}

/**
 * platform_init_boot_cpu initializes bare minimum to setup VM.
 */
void platform_init_boot_cpu()
{
    core_setup();
    interrupts_setup();
}

void platform_setup_boot_cpu()
{
    gic_setup();
    fpu_install();
}

void platform_setup_secondary_cpu()
{
}