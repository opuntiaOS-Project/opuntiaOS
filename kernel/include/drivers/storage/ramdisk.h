/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_STORAGE_RAMDISK_H
#define _KERNEL_DRIVERS_STORAGE_RAMDISK_H

#include <drivers/driver_manager.h>
#include <libkern/types.h>

void ramdisk_install();

#endif //_KERNEL_DRIVERS_STORAGE_RAMDISK_H