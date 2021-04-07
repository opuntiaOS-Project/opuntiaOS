/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_X86_MOUSE_H
#define _KERNEL_DRIVERS_X86_MOUSE_H

#include <drivers/generic/mouse.h>
#include <drivers/x86/ata.h>
#include <libkern/types.h>
#include <platform/x86/idt.h>

bool mouse_install();

#endif // _KERNEL_DRIVERS_X86_MOUSE_H
