/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_X86_BGA_H
#define _KERNEL_DRIVERS_X86_BGA_H

#include <drivers/driver_manager.h>
#include <drivers/x86/display.h>
#include <libkern/types.h>
#include <mem/kmalloc.h>
#include <platform/x86/port.h>

void bga_install();
void bga_init();
void bga_set_resolution(uint16_t width, uint16_t height);

#endif //_KERNEL_DRIVERS_X86_BGA_H
