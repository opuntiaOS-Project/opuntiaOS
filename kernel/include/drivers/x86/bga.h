/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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
int bga_init_with_dev(device_t* dev);
void bga_set_resolution(uint16_t width, uint16_t height);

#endif //_KERNEL_DRIVERS_X86_BGA_H
