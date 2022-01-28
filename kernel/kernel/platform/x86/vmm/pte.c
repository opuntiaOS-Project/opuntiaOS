/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <mem/vmm.h>
#include <platform/x86/vmm/pte.h>

void page_desc_init(page_desc_t* pte)
{
    *pte = 0;
}

uint32_t page_desc_mmu_flags_to_arch_flags(uint32_t attrs)
{
    return attrs;
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
    uint32_t res = MMU_FLAG_PERM_READ;
    if (page_desc_is_writable(pte)) {
        res |= MMU_FLAG_PERM_WRITE;
    }
    if (page_desc_is_user(pte)) {
        res |= MMU_FLAG_NONPRIV;
    }
    if (page_desc_is_not_cacheable(pte)) {
        res |= MMU_FLAG_UNCACHED;
    }
    if (page_desc_is_cow(pte)) {
        res |= MMU_FLAG_COW;
    }
    return res;
}

uint32_t page_desc_get_settings_ignore_cow(page_desc_t pte)
{
    uint32_t res = MMU_FLAG_PERM_READ;
    if (page_desc_is_writable(pte)) {
        res |= MMU_FLAG_PERM_WRITE;
    }
    if (page_desc_is_user(pte)) {
        res |= MMU_FLAG_NONPRIV;
    }
    if (page_desc_is_not_cacheable(pte)) {
        res |= MMU_FLAG_UNCACHED;
    }
    return res;
}