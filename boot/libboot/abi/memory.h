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

enum MEMORY_LAYOUT_FLAGS {
    MEMORY_LAYOUT_FLAG_TERMINATE = (1 << 0),
};

struct memory_layout {
    uint64_t base;
    uint64_t size;
    uint32_t flags;
};
typedef struct memory_layout memory_layout_t;

struct memory_boot_desc {
    uint64_t ram_base;
    uint64_t ram_size;
    memory_layout_t* reserved_areas;
};
typedef struct memory_boot_desc memory_boot_desc_t;

struct fb_boot_desc {
    uintptr_t vaddr;
    uintptr_t paddr;
    size_t width;
    size_t height;
    size_t pixels_per_row;
};
typedef struct fb_boot_desc fb_boot_desc_t;

struct boot_args {
    size_t paddr;
    size_t vaddr;
    size_t kernel_data_size;
    memory_boot_desc_t mem_boot_desc;
    fb_boot_desc_t fb_boot_desc;
    void* devtree;
    char cmd_args[32];
    char init_process[32];
};
typedef struct boot_args boot_args_t;

#endif // _BOOT_LIBBOOT_ABI_MEMORY_H