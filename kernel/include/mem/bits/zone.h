/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_BITS_ZONE_H
#define _KERNEL_MEM_BITS_ZONE_H

#include <mem/bits/mmu.h>

enum ZONE_TYPES {
    ZONE_TYPE_NULL = 0x0,
    ZONE_TYPE_CODE = 0x1,
    ZONE_TYPE_DATA = 0x2,
    ZONE_TYPE_STACK = 0x4,
    ZONE_TYPE_BSS = 0x8,
    ZONE_TYPE_DEVICE = 0x10,
    ZONE_TYPE_MAPPED = 0x20,
    ZONE_TYPE_MAPPED_FILE_PRIVATLY = 0x40,
    ZONE_TYPE_MAPPED_FILE_SHAREDLY = 0x80,
};

#endif // _KERNEL_MEM_BITS_ZONE_H