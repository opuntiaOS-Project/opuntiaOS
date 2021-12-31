/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/x86/ata.h>
#include <drivers/x86/bga.h>
#include <drivers/x86/display.h>
#include <drivers/x86/fpu.h>
#include <drivers/x86/ide.h>
#include <drivers/x86/keyboard.h>
#include <drivers/x86/mouse.h>
#include <drivers/x86/pci.h>
#include <drivers/x86/pit.h>
#include <platform/x86/gdt.h>
#include <platform/x86/idt.h>
#include <platform/x86/init.h>

void platform_init_boot_cpu()
{
    gdt_setup();
    interrupts_setup();
}

void platform_setup_boot_cpu()
{
    clean_screen();
    pit_setup();
    fpu_init();
}

void platform_setup_secondary_cpu()
{
}

void platform_drivers_setup()
{
    pci_install();
    ide_install();
    ata_install();
    kbdriver_install();
    mouse_install();
    bga_install();
}
