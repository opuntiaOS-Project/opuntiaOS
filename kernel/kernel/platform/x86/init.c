/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/timer/x86/pit.h>
#include <platform/x86/cpuinfo.h>
#include <platform/x86/fpu/fpu.h>
#include <platform/x86/gdt.h>
#include <platform/x86/idt.h>
#include <platform/x86/init.h>

void platform_init_boot_cpu()
{
    cpuinfo_init();
    gdt_setup();
    interrupts_setup();
}

void platform_setup_boot_cpu()
{
    pit_setup();
    fpu_init();
}

void platform_setup_secondary_cpu()
{
}
