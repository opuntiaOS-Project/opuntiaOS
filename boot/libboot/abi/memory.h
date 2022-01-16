/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_ABI_MEMORY_H
#define _BOOT_LIBBOOT_ABI_MEMORY_H

#include <libboot/types.h>

struct memory_map {
    uint32_t startLo;
    uint32_t startHi;
    uint32_t sizeLo;
    uint32_t sizeHi;
    uint32_t type;
    uint32_t acpi_3_0;
};
typedef struct memory_map memory_map_t;

struct mem_desc {
    uint16_t memory_map_size;
    uint16_t kernel_size;
};
typedef struct mem_desc mem_desc_t;

struct boot_desc {
    size_t paddr;
    size_t vaddr;
    void* memory_map;
    size_t memory_map_size;
    size_t kernel_size;
    void* devtree;
};
typedef struct boot_desc boot_desc_t;

#endif // _BOOT_LIBBOOT_ABI_MEMORY_H