/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/bitmap.h>
#include <libkern/bits/errno.h>
#include <mem/kmalloc.h>

#define BITMAP_BLOCKS_PER_BYTE (8)

bitmap_t bitmap_wrap(uint8_t* data, uint32_t len)
{
    bitmap_t bitmap;
    bitmap.data = data;
    bitmap.len = len;
    return bitmap;
}

/* FIXME: Let user know if alloction was unsucessful */
bitmap_t bitmap_allocate(uint32_t len)
{
    uint32_t alloc_len = len / BITMAP_BLOCKS_PER_BYTE;
    if (len % BITMAP_BLOCKS_PER_BYTE) {
        alloc_len++;
    }
    uint8_t* data = kmalloc(alloc_len);
    return bitmap_wrap(data, len);
}

/* TODO: can be speeded up with tzcnt */
int bitmap_find_space(bitmap_t bitmap, int req)
{
    int taken = 0;
    int start = 0;
    for (int i = 0; i < bitmap.len; i++) {
        if (bitmap.data[i] == 0xff) {
            taken = 0;
            continue;
        }
        for (int j = 0; j < BITMAP_BLOCKS_PER_BYTE; j++) {
            if ((bitmap.data[i] >> j) & 1) {
                taken = 0;
            } else {
                if (taken == 0) {
                    start = i * BITMAP_BLOCKS_PER_BYTE + j;
                }
                taken++;
                if (taken == req) {
                    return start;
                }
            }
        }
    }
    return -ENODATA;
}

int bitmap_find_space_aligned(bitmap_t bitmap, int req, int alignment)
{
    int taken = 0;
    int start = 0;
    for (int i = 0; i < bitmap.len; i++) {
        if (bitmap.data[i] == 0xff) {
            taken = 0;
            continue;
        }
        for (int j = 0; j < BITMAP_BLOCKS_PER_BYTE; j++) {
            if ((bitmap.data[i] >> j) & 1) {
                taken = 0;
            } else {
                if (taken == 0) {
                    start = i * BITMAP_BLOCKS_PER_BYTE + j;
                }
                if ((start % alignment) == 0) {
                    taken++;
                }
                if (taken == req) {
                    return start;
                }
            }
        }
    }
    return -ENODATA;
}

int bitmap_set(bitmap_t bitmap, int where)
{
    if (where >= bitmap.len * BITMAP_BLOCKS_PER_BYTE) {
        return -EFAULT;
    }

    int block = where / BITMAP_BLOCKS_PER_BYTE;
    int offset = where % BITMAP_BLOCKS_PER_BYTE;
    bitmap.data[block] |= (1 << offset);

    return 0;
}

int bitmap_unset(bitmap_t bitmap, int where)
{
    if (where >= bitmap.len * BITMAP_BLOCKS_PER_BYTE) {
        return -EFAULT;
    }

    int block = where / BITMAP_BLOCKS_PER_BYTE;
    int offset = where % BITMAP_BLOCKS_PER_BYTE;
    bitmap.data[block] &= ~((1 << offset));

    return 0;
}

int bitmap_set_range(bitmap_t bitmap, int start, int len)
{
    if (start + len - 1 >= bitmap.len * BITMAP_BLOCKS_PER_BYTE) {
        return -EFAULT;
    }

    int where = start;

    while (len && (where % BITMAP_BLOCKS_PER_BYTE) != 0) {
        bitmap_set(bitmap, where);
        where++;
        len--;
    }

    while (len >= BITMAP_BLOCKS_PER_BYTE) {
        int block = where / BITMAP_BLOCKS_PER_BYTE;
        bitmap.data[block] = 0xff;
        where += BITMAP_BLOCKS_PER_BYTE;
        len -= BITMAP_BLOCKS_PER_BYTE;
    }

    while (len) {
        bitmap_set(bitmap, where);
        where++;
        len--;
    }

    return 0;
}

int bitmap_unset_range(bitmap_t bitmap, int start, int len)
{
    if (start + len - 1 >= bitmap.len * BITMAP_BLOCKS_PER_BYTE) {
        return -EFAULT;
    }

    int where = start;

    while (len && (where % BITMAP_BLOCKS_PER_BYTE) != 0) {
        bitmap_unset(bitmap, where);
        where++;
        len--;
    }

    while (len >= BITMAP_BLOCKS_PER_BYTE) {
        int block = where / BITMAP_BLOCKS_PER_BYTE;
        bitmap.data[block] = 0x0;
        where += BITMAP_BLOCKS_PER_BYTE;
        len -= BITMAP_BLOCKS_PER_BYTE;
    }

    while (len) {
        bitmap_unset(bitmap, where);
        where++;
        len--;
    }

    return 0;
}