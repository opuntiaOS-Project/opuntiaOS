/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/log.h>
#include <libkern/types.h>
#include <mem/vmm/vmm.h>
#include <platform/aarch32/vmm/pte.h>

void page_desc_init(page_desc_t* pte)
{
    pte->data = 0;
    pte->one = 1;
    pte->ap1 = 0b01; // Kernel -- R/W and User -- No access
    pte->ap2 = 0;
    pte->c = 1;
    pte->s = 1;
    pte->b = 1;
    pte->tex = 0b001;
}

void page_desc_set_attrs(page_desc_t* pte, uint32_t attrs)
{
    if ((attrs & PAGE_DESC_PRESENT) == PAGE_DESC_PRESENT) {
        pte->one = 1;
    }
    if ((attrs & PAGE_DESC_USER) == PAGE_DESC_USER) {
        pte->ap1 = 0b10 | page_desc_is_writable(*pte);
    }
    if ((attrs & PAGE_DESC_WRITABLE) == PAGE_DESC_WRITABLE) {
        pte->ap1 |= 0b01;
    }
    if ((attrs & PAGE_DESC_NOT_CACHEABLE) == PAGE_DESC_NOT_CACHEABLE) {
        pte->c = 0;
    }
}

void page_desc_del_attrs(page_desc_t* pte, uint32_t attrs)
{
    if ((attrs & PAGE_DESC_PRESENT) == PAGE_DESC_PRESENT) {
        pte->one = 0;
    }
    if ((attrs & PAGE_DESC_WRITABLE) == PAGE_DESC_WRITABLE) {
        pte->ap1 = 0b10;
    }
    if ((attrs & PAGE_DESC_USER) == PAGE_DESC_USER) {
        pte->ap1 = 0b01;
    }
    if ((attrs & PAGE_DESC_NOT_CACHEABLE) == PAGE_DESC_NOT_CACHEABLE) {
        pte->c = 1;
    }
}

bool page_desc_has_attrs(page_desc_t pte, uint32_t attrs)
{
    if ((attrs & PAGE_DESC_PRESENT) == PAGE_DESC_PRESENT) {
        if (pte.one == 0) {
            return false;
        }
    }
    if ((attrs & PAGE_DESC_WRITABLE) == PAGE_DESC_WRITABLE) {
        if (pte.ap1 != 0b01) {
            return false;
        }
    }
    if ((attrs & PAGE_DESC_USER) == PAGE_DESC_USER) {
        if (pte.ap1 == 0b01) {
            return false;
        }
    }
    return true;
}

void page_desc_set_frame(page_desc_t* pte, uint32_t paddr)
{
    page_desc_del_frame(pte);
    pte->baddr = (paddr >> PAGE_DESC_FRAME_OFFSET);
}

void page_desc_del_frame(page_desc_t* pte)
{
    pte->baddr = 0;
}

bool page_desc_is_present(page_desc_t pte)
{
    return (pte.one != 0);
}

bool page_desc_is_writable(page_desc_t pte)
{
    return page_desc_has_attrs(pte, PAGE_DESC_WRITABLE);
}

bool page_desc_is_user(page_desc_t pte)
{
    return page_desc_has_attrs(pte, PAGE_DESC_WRITABLE);
}

bool page_desc_is_not_cacheable(page_desc_t pte)
{
    return (pte.c == 0);
}

uint32_t page_desc_get_frame(page_desc_t pte)
{
    return (pte.baddr << PAGE_DESC_FRAME_OFFSET);
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