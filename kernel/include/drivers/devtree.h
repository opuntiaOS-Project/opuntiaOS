/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_DEVTREE_H
#define _KERNEL_DRIVERS_DEVTREE_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>

#define DEVTREE_HEADER_SIGNATURE ("odtr3")
#define DEVTREE_HEADER_SIGNATURE_LEN (sizeof(DEVTREE_HEADER_SIGNATURE) - 1)

struct PACKED devtree_header {
    char signature[8];
    uint32_t revision;
    uint32_t flags;
    uint32_t entries_count;
    uint32_t name_list_offset;
};
typedef struct devtree_header devtree_header_t;

#define DEVTREE_ENTRY_FLAGS_MMIO = (1 << 0)
#define DEVTREE_ENTRY_TYPE_IO (0)
#define DEVTREE_ENTRY_TYPE_FB (1)
#define DEVTREE_ENTRY_TYPE_UART (2)
#define DEVTREE_ENTRY_TYPE_RAM (3)

struct PACKED devtree_entry {
    uint32_t type;
    uint32_t flags;
    uint32_t paddr;
    uint32_t rel_name_offset;
};
typedef struct devtree_entry devtree_entry_t;

int devtree_init(void* devtree);
devtree_entry_t* devtree_find_device(const char* name);

#endif // _KERNEL_DRIVERS_DEVTREE_H
