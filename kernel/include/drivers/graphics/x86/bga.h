/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_GRAPHICS_X86_BGA_H
#define _KERNEL_DRIVERS_GRAPHICS_X86_BGA_H

#include <drivers/driver_manager.h>
#include <libkern/types.h>
#include <mem/kmalloc.h>
#include <platform/x86/port.h>

void bga_install();
int bga_init_with_dev(device_t* dev);

#endif //_KERNEL_DRIVERS_GRAPHICS_X86_BGA_H
