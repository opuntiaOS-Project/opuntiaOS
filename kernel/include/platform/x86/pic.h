/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_PIC_H
#define _KERNEL_PLATFORM_X86_PIC_H

#include <platform/x86/port.h>

#define MASTER_PIC_CMD  0x0020
#define MASTER_PIC_DATA 0x0021
#define SLAVE_PIC_CMD   0x00A0
#define SLAVE_PIC_DATA  0x00A1
#define ICW4_8086	    0x01

void pic_remap(unsigned int offset1, unsigned int offset2);

#endif