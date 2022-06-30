/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_ARM64_VMM_CONSTS_H
#define _KERNEL_PLATFORM_ARM64_VMM_CONSTS_H

#include <libkern/types.h>

static inline int vm_page_size()
{
    return 0x1000;
}

static inline int vm_page_mask()
{
    return 0xfff;
}

#define VMM_LV0_ENTITY_COUNT (512)
#define VMM_LV1_ENTITY_COUNT (512)
#define VMM_LV2_ENTITY_COUNT (512)
#define VMM_LV3_ENTITY_COUNT (512)
#define VMM_PAGE_SIZE (vm_page_size())

#define PAGE_START(vaddr) ((vaddr & (~(uintptr_t)vm_page_mask())))
#define FRAME(addr) (addr / VMM_PAGE_SIZE)

#define PTABLE_LV_TOP (2)
#define PTABLE_LV0_VADDR_OFFSET (12)
#define PTABLE_LV1_VADDR_OFFSET (21)
#define PTABLE_LV2_VADDR_OFFSET (30)
#define PTABLE_LV3_VADDR_OFFSET (39)

#define VMM_USER_TABLES_START 0
#define VMM_KERNEL_TABLES_START VMM_LV0_ENTITY_COUNT

#define USER_HIGH 0x1fffffffff
#define KERNEL_BASE 0xffffff8000000000
#define KERNEL_PADDR_BASE 0xffffffff00000000 // up to 4gbs are supported.
#define KERNEL_KASAN_BASE 0xfffffff000000000
#define KERNEL_KASAN_SIZE (128 << 20) // 128MB for kasan covers 1GB of kernel space. For current need this is more than enough.

// For Apl
// static inline int get_page_size()
// {
//     return 0x4000;
// }

// static inline int get_page_mask()
// {
//     return 0x3fff;
// }

// #define VMM_LV0_ENTITY_COUNT (2048)
// #define VMM_LV1_ENTITY_COUNT (2048)
// #define VMM_LV2_ENTITY_COUNT (2048)
// #define VMM_LV3_ENTITY_COUNT (2)
// #define VMM_PAGE_SIZE (get_page_size())

// #define PAGE_START(vaddr) ((vaddr & ~(uintptr_t)get_page_mask())
// #define FRAME(addr) (addr / VMM_PAGE_SIZE)

// #define PTABLE_LV_TOP (3)
// #define PTABLE_LV0_VADDR_OFFSET (14)
// #define PTABLE_LV1_VADDR_OFFSET (25)
// #define PTABLE_LV2_VADDR_OFFSET (36)
// #define PTABLE_LV3_VADDR_OFFSET (47)

#endif //_KERNEL_PLATFORM_ARM64_VMM_CONSTS_H
