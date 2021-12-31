/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_X86_IDE_H
#define _KERNEL_DRIVERS_X86_IDE_H

#include <drivers/x86/ata.h>
#include <libkern/types.h>
#include <platform/x86/idt.h>

void ide_install();
void ide_find_devices(device_t* t_device);

#endif // _KERNEL_DRIVERS_X86_IDE_H
