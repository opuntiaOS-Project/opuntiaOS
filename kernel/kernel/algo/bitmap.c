/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/bitmap.h>
#include <libkern/bits/errno.h>
#include <libkern/platform.h>
#include <mem/kmalloc.h>

#define BITMAP_BLOCKS_PER_BYTE (8)

#define bitmap_get(bitmap, where) ((bitmap.data[where / BITMAP_BLOCKS_PER_BYTE] >> (where % BITMAP_BLOCKS_PER_BYTE)) & 1)

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

int bitmap_next_range_of_unset_bits(bitmap_t bitmap, int from, size_t min_len, int* start_of_free_chunks)
{
    uint32_t* bitmap32 = (uint32_t*)bitmap.data;

    // Calculating the start offset.
    int start_bucket_index = from / 32;
    int start_bucket_bit = from % 32;

    size_t free_chunks = 0;

    for (size_t bucket_index = start_bucket_index; bucket_index < bitmap.len / 32; bucket_index++) {
        if (bitmap32[bucket_index] == 0xffffffff) {
            // Skip over completely full bucket of size 32.
            if (free_chunks >= min_len) {
                return free_chunks;
            }
            free_chunks = 0;
            start_bucket_bit = 0;
            continue;
        }
        if (bitmap32[bucket_index] == 0x0) {
            // Skip over completely empty bucket of size 32.
            if (free_chunks == 0) {
                *start_of_free_chunks = bucket_index * 32;
            }
            free_chunks += 32;
            if (free_chunks >= min_len) {
                return free_chunks;
            }
            start_bucket_bit = 0;
            continue;
        }

        uint32_t bucket = bitmap32[bucket_index];
        int viewed_bits = start_bucket_bit;
        uint32_t trailing_zeroes = 0;

        bucket >>= viewed_bits;
        start_bucket_bit = 0;

        while (viewed_bits < 32) {
            if (bucket == 0) {
                if (free_chunks == 0) {
                    *start_of_free_chunks = bucket_index * 32 + viewed_bits;
                }
                free_chunks += 32 - viewed_bits;
                viewed_bits = 32;
            } else {
                trailing_zeroes = ctz32(bucket);
                bucket >>= trailing_zeroes;

                if (free_chunks == 0) {
                    *start_of_free_chunks = bucket_index * 32 + viewed_bits;
                }
                free_chunks += trailing_zeroes;
                viewed_bits += trailing_zeroes;

                if (free_chunks >= min_len) {
                    return free_chunks;
                }

                // Deleting trailing ones.
                uint32_t trailing_ones = ctz32(~bucket);
                bucket >>= trailing_ones;
                viewed_bits += trailing_ones;
                free_chunks = 0;
            }
        }
    }

    if (free_chunks < min_len) {
        size_t first_trailing_bit = (bitmap.len / 32) * 32;
        size_t trailing_bits = bitmap.len % 32;
        for (size_t i = 0; i < trailing_bits; i++) {
            if (!bitmap_get(bitmap, first_trailing_bit + i)) {
                if (!free_chunks) {
                    *start_of_free_chunks = first_trailing_bit + i;
                }
                if (++free_chunks >= min_len) {
                    return free_chunks;
                }
            } else {
                free_chunks = 0;
            }
        }
        return -1;
    }

    return free_chunks;
}

int bitmap_find_space(bitmap_t bitmap, int req)
{
    int start = 0;
    int len = bitmap_next_range_of_unset_bits(bitmap, 0, req, &start);
    if (len < 0) {
        return -ENODATA;
    }

    return start;
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