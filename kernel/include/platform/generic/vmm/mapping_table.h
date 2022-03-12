/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_GENERIC_VMM_MAPPING_TABLE_H
#define _KERNEL_PLATFORM_GENERIC_VMM_MAPPING_TABLE_H

#include <libkern/types.h>

struct mapping_entry {
    uint32_t paddr;
    uint32_t vaddr;
    uint32_t flags;
    uint16_t pages;
    uint16_t last; // 1 if an element is the last.
};
typedef struct mapping_entry mapping_entry_t;

extern mapping_entry_t extern_mapping_table[]; // Maps after kernel tables are ready, so can be outside kernelspace

#endif // _KERNEL_PLATFORM_GENERIC_VMM_MAPPING_TABLE_H
