/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// #include <platform/riscv64/fpu/fpu.h>
#include <platform/riscv64/init.h>
#include <platform/riscv64/interrupts.h>

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
    plic_setup();
    // fpu_install();
}

void platform_setup_secondary_cpu()
{
}