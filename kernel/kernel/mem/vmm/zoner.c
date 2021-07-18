/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
 *  Other data
 */

#include <algo/bitmap.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/lock.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>

#define ZONER_BITMAP_SIZE (4 * 1024 * 8)
#define ZONER_TO_BITMAP_INDEX(x) ((x - KERNEL_BASE) >> 12)
#define ZONER_FROM_BITMAP_INDEX(x) ((x << 12) + KERNEL_BASE)

static uint32_t _zoner_next_vaddr;
static uint8_t* _zoner_bitmap;
static lock_t _zoner_lock;
static bitmap_t bitmap;
static bool _zoner_bitmap_set;

/**
 * The function is used to allocate zones before bitmap is set.
 */
static uint32_t _zoner_new_vzone_lockless(uint32_t size)
{
    uint32_t res = _zoner_next_vaddr;
    _zoner_next_vaddr += size;
    return res;
}

static uint32_t _zoner_new_vzone_aligned_lockless(uint32_t size, uint32_t alignment)
{
    uint32_t res = _zoner_next_vaddr;
    _zoner_next_vaddr += size + alignment;
    res += alignment;
    res = res - (res % alignment);
    return res;
}

/**
 * zoner_place_bitmap places bitmap at _zoner_start_vaddr.
 */
void zoner_place_bitmap()
{
    lock_acquire(&_zoner_lock);
    _zoner_bitmap = (uint8_t*)_zoner_new_vzone_lockless(ZONER_BITMAP_SIZE);
    bitmap = bitmap_wrap(_zoner_bitmap, ZONER_BITMAP_SIZE * 8);
    memset(_zoner_bitmap, 0, ZONER_BITMAP_SIZE);
    _zoner_bitmap_set = true;
    bitmap_set_range(bitmap, 0, ZONER_TO_BITMAP_INDEX(_zoner_next_vaddr));
    lock_release(&_zoner_lock);
}

void zoner_init(uint32_t start_vaddr)
{
    lock_init(&_zoner_lock);
    _zoner_next_vaddr = start_vaddr;
}

/**
 * Returns new zone vaddr start.
 * Note, the function does NOT map this vaddr, it's on your own.
 */
zone_t zoner_new_zone(uint32_t size)
{
    lock_acquire(&_zoner_lock);
    if (size % VMM_PAGE_SIZE) {
        size += VMM_PAGE_SIZE - (size % VMM_PAGE_SIZE);
    }

    zone_t zone;

    if (!_zoner_bitmap_set) {
        zone.start = _zoner_new_vzone_lockless(size);
    } else {
        uint32_t blocks = size / VMM_PAGE_SIZE;
        int start = bitmap_find_space(bitmap, blocks);

        if (start < 0) {
            zone.start = 0;
            zone.len = 0;
            lock_release(&_zoner_lock);
            return zone;
        }

        if (bitmap_set_range(bitmap, start, blocks) < 0) {
            zone.start = 0;
        } else {
            zone.start = ZONER_FROM_BITMAP_INDEX(start);
        }
    }

    zone.len = size;
    lock_release(&_zoner_lock);
    return zone;
}

zone_t zoner_new_zone_aligned(uint32_t size, uint32_t alignment)
{
    lock_acquire(&_zoner_lock);
    if (size % VMM_PAGE_SIZE) {
        size += VMM_PAGE_SIZE - (size % VMM_PAGE_SIZE);
    }

    if (alignment % VMM_PAGE_SIZE) {
        alignment += VMM_PAGE_SIZE - (alignment % VMM_PAGE_SIZE);
    }

    zone_t zone;

    if (!_zoner_bitmap_set) {
        zone.start = _zoner_new_vzone_aligned_lockless(size, alignment);
    } else {
        uint32_t blocks = size / VMM_PAGE_SIZE;
        uint32_t blocks_alignment = alignment / VMM_PAGE_SIZE;
        int start = bitmap_find_space_aligned(bitmap, blocks, blocks_alignment);

        if (start < 0) {
            zone.start = 0;
            zone.len = 0;
            lock_release(&_zoner_lock);
            return zone;
        }

        if (bitmap_set_range(bitmap, start, blocks) < 0) {
            zone.start = 0;
        } else {
            zone.start = ZONER_FROM_BITMAP_INDEX(start);
        }
    }

    zone.len = size;
    lock_release(&_zoner_lock);
    return zone;
}

/**
 * Returns new zone vaddr start.
 * Note, the function does NOT map this vaddr, it's on your own.
 */
static ALWAYS_INLINE int zoner_free_zone_lockless(zone_t zone)
{
    /* Checking if it was allocated with bitmap */
    if (zone.start < _zoner_next_vaddr) {
        return -EPERM;
    }

    int start = ZONER_TO_BITMAP_INDEX(zone.start);
    uint32_t blocks = zone.len / VMM_PAGE_SIZE;
    return bitmap_unset_range(bitmap, start, blocks);
}

int zoner_free_zone(zone_t zone)
{
    lock_acquire(&_zoner_lock);
    int res = zoner_free_zone_lockless(zone);
    lock_release(&_zoner_lock);
    return res;
}