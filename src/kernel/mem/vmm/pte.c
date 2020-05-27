/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <mem/vmm/pte.h>

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
    return (pte & attrs > 0);
}

void page_desc_set_frame(page_desc_t* pte, uint32_t frame)
{
    page_desc_del_frame(pte);
    *pte |= (frame << PAGE_DESC_FRAME_OFFSET);
}

void page_desc_del_frame(page_desc_t* pte)
{
    *pte &= ((1 << (PAGE_DESC_FRAME_OFFSET)) - 1);
}

bool page_desc_is_present(page_desc_t pte)
{
    return (pte & PAGE_DESC_PRESENT > 0);
}

bool page_desc_is_writable(page_desc_t pte)
{
    return (pte & PAGE_DESC_WRITABLE > 0);
}

bool page_desc_is_user(page_desc_t pte)
{
    return (pte & PAGE_DESC_USER > 0);
}

uint32_t page_desc_get_frame(page_desc_t pte)
{
    return (pte >> PAGE_DESC_FRAME_OFFSET) << PAGE_DESC_FRAME_OFFSET;
}
