/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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
#include <mem/kmemzone.h>
#include <mem/vmm.h>

#define ZONER_BITMAP_SIZE (4 * 1024 * 8)
#define ZONER_TO_BITMAP_INDEX(x) ((x - KERNEL_BASE) >> 12)
#define ZONER_FROM_BITMAP_INDEX(x) ((x << 12) + KERNEL_BASE)

static uintptr_t _zoner_next_vaddr;
static uint8_t* _zoner_bitmap;
static spinlock_t _zoner_lock;
static bitmap_t bitmap;
static bool _zoner_bitmap_set;

/**
 * The function is used to allocate zones before bitmap is set.
 */
static uintptr_t _zoner_new_vzone_locked(size_t size)
{
    uintptr_t res = _zoner_next_vaddr;
    _zoner_next_vaddr += size;
    return res;
}

static uintptr_t _zoner_new_vzone_aligned_locked(size_t size, size_t alignment)
{
    uintptr_t res = _zoner_next_vaddr;
    _zoner_next_vaddr += size + alignment;
    res += alignment;
    res = res - (res % alignment);
    return res;
}

/**
 * kmemzone_init_stage2 places bitmap at _zoner_start_vaddr.
 */
void kmemzone_init_stage2()
{
    spinlock_acquire(&_zoner_lock);
    _zoner_bitmap = (uint8_t*)_zoner_new_vzone_locked(ZONER_BITMAP_SIZE);
    bitmap = bitmap_wrap(_zoner_bitmap, ZONER_BITMAP_SIZE * 8);
    memset(_zoner_bitmap, 0, ZONER_BITMAP_SIZE);
    _zoner_bitmap_set = true;
    bitmap_set_range(bitmap, 0, ZONER_TO_BITMAP_INDEX(_zoner_next_vaddr));
    spinlock_release(&_zoner_lock);
}

void kmemzone_init()
{
    spinlock_init(&_zoner_lock);

    const pmm_state_t* pmm_state = pmm_get_state();
    uintptr_t start_vaddr = (uintptr_t)pmm_state->mat.data + pmm_state->mat.len / 8;

    // The initial start address is aligned by 1st level ptable, since
    // pspace32 expects to start at the beggining of the 1st level ptable.
    // 0x400000 as an alignment works for all 32bit targets (don't see any problems
    // for 64bit platforms either).
    start_vaddr = ROUND_CEIL(start_vaddr, 0x400000);

    _zoner_next_vaddr = start_vaddr;
}

/**
 * Returns new zone vaddr start.
 * Note, the function does NOT map this vaddr, it's on your own.
 */
kmemzone_t kmemzone_new(size_t size)
{
    spinlock_acquire(&_zoner_lock);
    if (size % VMM_PAGE_SIZE) {
        size += VMM_PAGE_SIZE - (size % VMM_PAGE_SIZE);
    }

    kmemzone_t zone;

    if (!_zoner_bitmap_set) {
        zone.start = _zoner_new_vzone_locked(size);
    } else {
        size_t blocks = size / VMM_PAGE_SIZE;
        int start = bitmap_find_space(bitmap, blocks);

        if (start < 0) {
            zone.start = 0;
            zone.len = 0;
            spinlock_release(&_zoner_lock);
            return zone;
        }

        if (bitmap_set_range(bitmap, start, blocks) < 0) {
            zone.start = 0;
        } else {
            zone.start = ZONER_FROM_BITMAP_INDEX(start);
        }
    }

    zone.len = size;
    spinlock_release(&_zoner_lock);
    return zone;
}

kmemzone_t kmemzone_new_aligned(size_t size, size_t alignment)
{
    spinlock_acquire(&_zoner_lock);
    if (size % VMM_PAGE_SIZE) {
        size += VMM_PAGE_SIZE - (size % VMM_PAGE_SIZE);
    }

    if (alignment % VMM_PAGE_SIZE) {
        alignment += VMM_PAGE_SIZE - (alignment % VMM_PAGE_SIZE);
    }

    kmemzone_t zone;

    if (!_zoner_bitmap_set) {
        zone.start = _zoner_new_vzone_aligned_locked(size, alignment);
    } else {
        size_t blocks = size / VMM_PAGE_SIZE;
        size_t blocks_alignment = alignment / VMM_PAGE_SIZE;
        int start = bitmap_find_space_aligned(bitmap, blocks, blocks_alignment);

        if (start < 0) {
            zone.start = 0;
            zone.len = 0;
            spinlock_release(&_zoner_lock);
            return zone;
        }

        if (bitmap_set_range(bitmap, start, blocks) < 0) {
            zone.start = 0;
        } else {
            zone.start = ZONER_FROM_BITMAP_INDEX(start);
        }
    }

    zone.len = size;
    spinlock_release(&_zoner_lock);
    return zone;
}

/**
 * Returns new zone vaddr start.
 * Note, the function does NOT map this vaddr, it's on your own.
 */
static ALWAYS_INLINE int kmemzone_free_locked(kmemzone_t zone)
{
    /* Checking if it was allocated with bitmap */
    if (zone.start < _zoner_next_vaddr) {
        return -EPERM;
    }

    int start = ZONER_TO_BITMAP_INDEX(zone.start);
    size_t blocks = zone.len / VMM_PAGE_SIZE;
    return bitmap_unset_range(bitmap, start, blocks);
}

int kmemzone_free(kmemzone_t zone)
{
    spinlock_acquire(&_zoner_lock);
    int res = kmemzone_free_locked(zone);
    spinlock_release(&_zoner_lock);
    return res;
}