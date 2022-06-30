/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_IO_X86_MOUSE_H
#define _KERNEL_DRIVERS_IO_X86_MOUSE_H

#include <drivers/io/mouse.h>
#include <libkern/types.h>

void mouse_install();

#endif // _KERNEL_DRIVERS_IO_X86_MOUSE_H
