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

#define page_desc_t uint32_t
#define pte_t uint32_t

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
    PAGE_DESC_ZEROING_ON_DEMAND = 0x800
};

void page_desc_init(page_desc_t* pte);
void page_desc_set_attrs(page_desc_t* pte, uint32_t attrs);
void page_desc_del_attrs(page_desc_t* pte, uint32_t attrs);
bool page_desc_has_attrs(page_desc_t pte, uint32_t attr);

void page_desc_set_frame(page_desc_t* pte, uint32_t frame);
void page_desc_del_frame(page_desc_t* pte);

bool page_desc_is_present(page_desc_t pte);
bool page_desc_is_writable(page_desc_t pte);
bool page_desc_is_user(page_desc_t pte);
bool page_desc_is_not_cacheable(page_desc_t pte);
bool page_desc_is_cow(page_desc_t pte);

uint32_t page_desc_get_frame(page_desc_t pte);
uint32_t page_desc_get_settings(page_desc_t pte);
uint32_t page_desc_get_settings_ignore_cow(page_desc_t pte);

#endif //_KERNEL_PLATFORM_X86_VMM_PTE_H
