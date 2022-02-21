/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_VMM_PTE_H
#define _KERNEL_PLATFORM_X86_VMM_PTE_H

#include <libkern/types.h>
#include <mem/bits/mmu.h>

#define PAGE_DESC_FRAME_OFFSET 12

enum PAGE_DESC_PAGE_FLAGS {
    PAGE_DESC_PRESENT = 0x1,
    PAGE_DESC_WRITABLE = 0x2,
    PAGE_DESC_USER = 0x4,
    PAGE_DESC_WRITETHOUGH = 0x8,
    PAGE_DESC_NOT_CACHEABLE = 0x10,
    PAGE_DESC_ACCESSED = 0x20,
    PAGE_DESC_DIRTY = 0x40,
    PAGE_DESC_PAT = 0x80,
    PAGE_DESC_CPU_GLOBAL = 0x100,
    PAGE_DESC_LV4_GLOBAL = 0x200,
    PAGE_DESC_COPY_ON_WRITE = 0x400,
};

#endif //_KERNEL_PLATFORM_X86_VMM_PTE_H
