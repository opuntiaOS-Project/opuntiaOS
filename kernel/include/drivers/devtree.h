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
#include <mem/boot.h>

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
#define DEVTREE_ENTRY_TYPE_STORAGE (4)
#define DEVTREE_ENTRY_TYPE_BUS_CONTROLLER (5)
#define DEVTREE_ENTRY_TYPE_RTC (6)

struct PACKED devtree_entry {
    uint32_t type;
    uint32_t flags;
    uint64_t region_base;
    uint64_t region_size;
    uint32_t irq_lane;
    uint32_t irq_flags;
    uint32_t irq_priority;
    uint32_t rel_name_offset;
    uint64_t aux1;
    uint64_t aux2;
};
typedef struct devtree_entry devtree_entry_t;

int devtree_init(boot_args_t* boot_args);
devtree_entry_t* devtree_find_device(const char* name);
const char* devtree_name_of_entry(devtree_entry_t* en);
devtree_entry_t* devtree_new_entry(const devtree_entry_t* from);
uint32_t devtree_new_entry_name(const char* ptr);

#endif // _KERNEL_DRIVERS_DEVTREE_H
