/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

/**
 * Zone Manager is a helper module which split kernel space (3GB+) to zones.
 */

#include <mem/vmm/zonem.h>
#include <mem/vmm/vmm.h>

static uint32_t _zonem_start_vaddr;

void zonem_init(uint32_t start_vaddr)
{
    _zonem_start_vaddr = start_vaddr;
}

/**
 * Returns new zone vaddr start.
 * Note, the function does NOT map this vaddr, it's on your own.
 */
uint32_t zonem_new_vzone(uint32_t size)
{
    if (size % VMM_PAGE_SIZE) {
        size = (size + VMM_PAGE_SIZE) % VMM_PAGE_SIZE;
    }

    uint32_t res = _zonem_start_vaddr;
    _zonem_start_vaddr += size;
    return res;
}