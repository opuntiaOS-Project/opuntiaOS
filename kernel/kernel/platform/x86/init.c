/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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

void platform_setup()
{
    clean_screen();
    gdt_setup();
    interrupts_setup();
    pit_setup();
    fpu_init();
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
