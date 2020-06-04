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

#include <algo/bitmap.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>

#define ZONER_BITMAP_SIZE (4 * 1024 * 1024)
#define ZONER_TO_BITMAP_INDEX(x) (x >> 12)

static uint32_t _zoner_start_vaddr;
static uint8_t* _zoner_bitmap;
static bitmap_t bitmap;

void zoner_init(uint32_t start_vaddr)
{
    _zoner_start_vaddr = start_vaddr;
    // bitmap = bitmap_create(_zoner_bitmap, ZONER_BITMAP_LEN);
    // memset(_zoner_bitmap, 0, ZONER_BITMAP_LEN);
}

/**
 * Returns new zone vaddr start.
 * Note, the function does NOT map this vaddr, it's on your own.
 */
uint32_t zoner_new_vzone(uint32_t size)
{
    if (size % VMM_PAGE_SIZE) {
        size = (size + VMM_PAGE_SIZE) % VMM_PAGE_SIZE;
    }

    uint32_t res = _zoner_start_vaddr;
    _zoner_start_vaddr += size;
    return res;
}