/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_TARGET_CORTEX_A15_MEMMAP_H
#define _BOOT_TARGET_CORTEX_A15_MEMMAP_H

#include <libboot/abi/memory.h>

extern memory_map_t arm_memmap[2];

memory_map_t* memmap_init();
static inline size_t memmap_size()
{
    return sizeof(arm_memmap);
}

#endif // _BOOT_TARGET_CORTEX_A15_MEMMAP_H