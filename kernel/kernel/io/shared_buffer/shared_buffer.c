/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/bitmap.h>
#include <io/shared_buffer/shared_buffer.h>
#include <libkern/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>

// #define SHARED_BUFFER_DEBUG

#define SHBUF_SPACE_SIZE (128 * MB)
#define SHBUF_BLOCK_SIZE (4 * KB)
#define SHBUF_MAX_BUFFERS 128

uint8_t* buffers[SHBUF_MAX_BUFFERS];

struct shared_buffer_header {
    size_t len;
};
typedef struct shared_buffer_header shared_buffer_header_t;

static zone_t _shared_buffer_zone;
static uint32_t _shared_buffer_bitmap_len = 0;
static uint8_t* _shared_buffer_bitmap;
static bitmap_t bitmap;

static inline uint32_t _shared_buffer_to_vaddr(int start)
{
    return (uint32_t)_shared_buffer_zone.start + start * SHBUF_BLOCK_SIZE;
}

static inline int _shared_buffer_to_index(uint32_t vaddr)
{
    return (vaddr - (uint32_t)_shared_buffer_zone.start) / SHBUF_BLOCK_SIZE;
}

static inline int _shared_buffer_alloc_id()
{
    for (int i = 0; i < SHBUF_MAX_BUFFERS; i++) {
        if (!buffers[i]) {
            return i;
        }
    }
    return -1;
}

static void _shared_buffer_init_bitmap()
{
    _shared_buffer_bitmap = _shared_buffer_zone.ptr;
    _shared_buffer_bitmap_len = (SHBUF_SPACE_SIZE / SHBUF_BLOCK_SIZE / 8);

    bitmap = bitmap_wrap(_shared_buffer_bitmap, _shared_buffer_bitmap_len);
    memset(_shared_buffer_bitmap, 0, _shared_buffer_bitmap_len);

    /* Setting bitmap as a busy region. */
    int blocks_needed = (_shared_buffer_bitmap_len + SHBUF_BLOCK_SIZE - 1) / SHBUF_BLOCK_SIZE;
    bitmap_set_range(bitmap, _shared_buffer_to_index((uint32_t)_shared_buffer_bitmap), blocks_needed);
}

int shared_buffer_init()
{
    _shared_buffer_zone = zoner_new_zone(SHBUF_SPACE_SIZE);
    _shared_buffer_init_bitmap();
    return 0;
}

int shared_buffer_create(uint8_t** res_buffer, size_t size)
{
    int buf_id = _shared_buffer_alloc_id();
    if (buf_id < 0) {
        return -ENOMEM;
    }

    size_t act_size = size + sizeof(shared_buffer_header_t);

    int blocks_needed = (act_size + SHBUF_BLOCK_SIZE - 1) / SHBUF_BLOCK_SIZE;
    int start = bitmap_find_space(bitmap, blocks_needed);
    if (start < 0) {
        return -ENOMEM;
    }

    shared_buffer_header_t* space = (shared_buffer_header_t*)_shared_buffer_to_vaddr(start);
    space->len = act_size;
    bitmap_set_range(bitmap, start, blocks_needed);
    vmm_tune_pages((uint32_t)space, act_size, PAGE_WRITABLE | PAGE_EXECUTABLE | PAGE_READABLE | PAGE_USER);

    *res_buffer = (uint8_t*)&space[1];
    buffers[buf_id] = (uint8_t*)&space[1];
#ifdef SHARED_BUFFER_DEBUG
    log("Buffer created at %x %d", buffers[buf_id], buf_id);
#endif
    return buf_id;
}

int shared_buffer_get(int id, uint8_t** res_buffer)
{
    if (unlikely(id < 0 || SHBUF_MAX_BUFFERS <= id)) {
        return -EINVAL;
    }
    if (unlikely(buffers[id] == 0)) {
        return -EINVAL;
    }
#ifdef SHARED_BUFFER_DEBUG
    log("Buffer opened at %x %d", buffers[id], id);
#endif
    *res_buffer = buffers[id];
    return 0;
}

int shared_buffer_free(int id)
{
    if (unlikely(id < 0 || SHBUF_MAX_BUFFERS <= id)) {
        return -EINVAL;
    }
    if (unlikely(buffers[id] == 0)) {
        return -EINVAL;
    }

    shared_buffer_header_t* sptr = (shared_buffer_header_t*)buffers[id];
    size_t blocks_to_delete = (sptr[-1].len + SHBUF_BLOCK_SIZE - 1) / SHBUF_BLOCK_SIZE;
    bitmap_unset_range(bitmap, _shared_buffer_to_index((uint32_t)&sptr[-1]), blocks_to_delete);
    buffers[id] = 0;
    return 0;
}