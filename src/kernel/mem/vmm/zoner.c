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

/**
 * Current distribution:
 *  Kernel      	4 MB
 *  Pspace      	4 MB
 *  Zoner Bitmap	32 KB
 *  Kmalloc Space	32Kb + 4 MB
 *  Syscall Jumper	4 KB
 */

#include <algo/bitmap.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>

#define ZONER_BITMAP_SIZE (4 * 1024 * 8)
#define ZONER_TO_BITMAP_INDEX(x) ((x - KERNEL_BASE) >> 12) 
#define ZONER_FROM_BITMAP_INDEX(x) ((x << 12) + KERNEL_BASE)

static uint32_t _zoner_next_vaddr;
static uint8_t* _zoner_bitmap;
static bitmap_t bitmap;
static bool _zoner_bitmap_set;

/**
 * The function is used to allocate zone before bitmap is set.
 */
static uint32_t _zoner_new_vzone(uint32_t size)
{
    uint32_t res = _zoner_next_vaddr;
    _zoner_next_vaddr += size;
    return res;
}

/**
 * zoner_place_bitmap places bitmap at _zoner_start_vaddr.
 */
void zoner_place_bitmap()
{
    _zoner_bitmap = (uint8_t*)_zoner_new_vzone(ZONER_BITMAP_SIZE);
    bitmap = bitmap_wrap(_zoner_bitmap, ZONER_BITMAP_SIZE * 8);
    memset(_zoner_bitmap, 0, ZONER_BITMAP_SIZE * 8);
    _zoner_bitmap_set = true;
    bitmap_set_range(bitmap, 0, ZONER_TO_BITMAP_INDEX(_zoner_next_vaddr));
}

void zoner_init(uint32_t start_vaddr)
{
    _zoner_next_vaddr = start_vaddr;
}

/**
 * Returns new zone vaddr start.
 * Note, the function does NOT map this vaddr, it's on your own.
 */
zone_t zoner_new_zone(uint32_t size)
{
    if (size % VMM_PAGE_SIZE) {
        size = (size + VMM_PAGE_SIZE) % VMM_PAGE_SIZE;
    }

    zone_t zone;

    if (!_zoner_bitmap_set) {
        zone.start = _zoner_new_vzone(size);
    } else {
        uint32_t blocks = size / VMM_PAGE_SIZE;
        int start = bitmap_find_space(bitmap, blocks);
        
        if (start < 0) {
            zone.start = 0;
            zone.len = 0;
            return zone;
        }

        if (bitmap_set_range(bitmap, start, blocks) < 0) {
            zone.start = 0;
        } else {
            zone.start = ZONER_FROM_BITMAP_INDEX(start);
        }
    }

    zone.len = size;
    return zone;
}

/**
 * Returns new zone vaddr start.
 * Note, the function does NOT map this vaddr, it's on your own.
 */
int zoner_free_zone(zone_t zone)
{
    /* Checking if it was allocated with bitmap */
    if (zone.start < _zoner_next_vaddr) {
        return -1;
    }

    int start = ZONER_TO_BITMAP_INDEX(zone.start);
    uint32_t blocks = zone.len / VMM_PAGE_SIZE;
    return bitmap_unset_range(bitmap, start, blocks);
}