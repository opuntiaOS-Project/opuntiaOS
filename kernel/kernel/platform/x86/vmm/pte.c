/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <platform/x86/vmm/pte.h>
#include <mem/vmm/vmm.h>

void page_desc_init(page_desc_t* pte)
{
    *pte = 0;
}

void page_desc_set_attrs(page_desc_t* pte, uint32_t attrs)
{
    *pte |= attrs;
}

void page_desc_del_attrs(page_desc_t* pte, uint32_t attrs)
{
    *pte &= ~(attrs);
}

bool page_desc_has_attrs(page_desc_t pte, uint32_t attrs)
{
    return ((pte & attrs) > 0);
}

void page_desc_set_frame(page_desc_t* pte, uint32_t frame)
{
    page_desc_del_frame(pte);
    frame >>= PAGE_DESC_FRAME_OFFSET;
    *pte |= (frame << PAGE_DESC_FRAME_OFFSET);
}

void page_desc_del_frame(page_desc_t* pte)
{
    *pte &= ((1 << (PAGE_DESC_FRAME_OFFSET)) - 1);
}

bool page_desc_is_present(page_desc_t pte)
{
    return ((pte & PAGE_DESC_PRESENT) > 0);
}

bool page_desc_is_writable(page_desc_t pte)
{
    return ((pte & PAGE_DESC_WRITABLE) > 0);
}

bool page_desc_is_user(page_desc_t pte)
{
    return ((pte & PAGE_DESC_USER) > 0);
}

bool page_desc_is_not_cacheable(page_desc_t pte)
{
    return ((pte & PAGE_DESC_NOT_CACHEABLE) > 0);
}

bool page_desc_is_cow(page_desc_t pte)
{
    return ((pte & PAGE_DESC_COPY_ON_WRITE) > 0);
}

uint32_t page_desc_get_frame(page_desc_t pte)
{
    return ((pte >> PAGE_DESC_FRAME_OFFSET) << PAGE_DESC_FRAME_OFFSET);
}

uint32_t page_desc_get_settings(page_desc_t pte)
{
    uint32_t res = PAGE_READABLE;
    if (page_desc_is_writable(pte)) {
        res |= PAGE_WRITABLE;
    }
    if (page_desc_is_user(pte)) {
        res |= PAGE_USER;
    }
    if (page_desc_is_not_cacheable(pte)) {
        res |= PAGE_NOT_CACHEABLE;
    }
    if (page_desc_is_cow(pte)) {
        res |= PAGE_COW;
    }
    return res;
}

uint32_t page_desc_get_settings_ignore_cow(page_desc_t pte)
{
    uint32_t res = PAGE_READABLE;
    if (page_desc_is_writable(pte)) {
        res |= PAGE_WRITABLE;
    }
    if (page_desc_is_user(pte)) {
        res |= PAGE_USER;
    }
    if (page_desc_is_not_cacheable(pte)) {
        res |= PAGE_NOT_CACHEABLE;
    }
    return res;
}