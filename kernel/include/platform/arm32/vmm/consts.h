/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_ARM32_VMM_CONSTS_H
#define _KERNEL_PLATFORM_ARM32_VMM_CONSTS_H

static inline int vm_page_mask()
{
    return 0xfff;
}

#define VMM_LV0_ENTITY_COUNT (256)
#define VMM_LV1_ENTITY_COUNT (4096)
#define VMM_PAGE_SIZE (4096)

#define VMM_OFFSET_IN_DIRECTORY(a) (((a) >> 20) & 0xfff)
#define VMM_OFFSET_IN_TABLE(a) (((a) >> 12) & 0xff)
#define VMM_OFFSET_IN_PAGE(a) ((a)&0xfff)
#define TABLE_START(vaddr) ((vaddr >> 20) << 20)
#define PAGE_START(vaddr) ((vaddr >> 12) << 12)
#define FRAME(addr) (addr / VMM_PAGE_SIZE)

#define PTABLE_TOP_KERNEL_OFFSET 3072

#define PTABLE_LV_TOP (1)
#define PTABLE_LV0_VADDR_OFFSET (12)
#define PTABLE_LV1_VADDR_OFFSET (20)

#define USER_HIGH 0xbfffffff
#define KERNEL_BASE 0xc0000000

#endif //_KERNEL_PLATFORM_ARM32_VMM_CONSTS_H
