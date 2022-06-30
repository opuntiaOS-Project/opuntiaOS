/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/aarch32/fpu/fpuv4.h>
#include <platform/aarch32/init.h>
#include <platform/aarch32/interrupts.h>
#include <platform/aarch32/target/cpu_part_numbers.h>
#include <platform/aarch32/target/general/core_setup.h>

int core_setup()
{
    int major_part = extract_bits(read_midr(), 4, 15);
    switch (major_part) {
    case PART_NUMBER_CORTEX_A15:
        return cortex_a15_setup();

    default:
        return 0;
    }
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
    fpuv4_install();
    gic_setup();
}

void platform_setup_secondary_cpu()
{
    interrupts_setup_secondary_cpu();
    fpuv4_install();
    gic_setup_secondary_cpu();
}