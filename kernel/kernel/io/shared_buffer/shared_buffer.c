/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/bitmap.h>
#include <io/shared_buffer/shared_buffer.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/lock.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>

// #define SHARED_BUFFER_DEBUG

#define SHBUF_SPACE_SIZE (128 * MB)
#define SHBUF_BLOCK_SIZE (4 * KB)
#define SHBUF_MAX_BUFFERS 128

uint8_t* buffers[SHBUF_MAX_BUFFERS];

struct shared_buffer_header {
    size_t len;
};
typedef struct shared_buffer_header shared_buffer_header_t;

static spinlock_t _shared_buffer_lock;
static kmemzone_t _shared_buffer_zone;
static size_t _shared_buffer_bitmap_len = 0;
static uint8_t* _shared_buffer_bitmap;
static bitmap_t bitmap;

static inline uintptr_t _shared_buffer_to_vaddr(int start)
{
    return (uintptr_t)_shared_buffer_zone.start + start * SHBUF_BLOCK_SIZE;
}

static inline int _shared_buffer_to_index(uintptr_t vaddr)
{
    return (vaddr - (uintptr_t)_shared_buffer_zone.start) / SHBUF_BLOCK_SIZE;
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
    bitmap_set_range(bitmap, _shared_buffer_to_index((uintptr_t)_shared_buffer_bitmap), blocks_needed);
}

int shared_buffer_init()
{
    _shared_buffer_zone = kmemzone_new(SHBUF_SPACE_SIZE);
    _shared_buffer_init_bitmap();
    spinlock_init(&_shared_buffer_lock);
    return 0;
}

int shared_buffer_create(uintptr_t __user* res_buffer, size_t size)
{
    spinlock_acquire(&_shared_buffer_lock);
    int buf_id = _shared_buffer_alloc_id();
    if (buf_id < 0) {
        spinlock_release(&_shared_buffer_lock);
        return -ENOMEM;
    }

    size_t act_size = size + sizeof(shared_buffer_header_t);

    int blocks_needed = (act_size + SHBUF_BLOCK_SIZE - 1) / SHBUF_BLOCK_SIZE;
    int start = bitmap_find_space(bitmap, blocks_needed);
    if (start < 0) {
        spinlock_release(&_shared_buffer_lock);
        return -ENOMEM;
    }

    shared_buffer_header_t* space = (shared_buffer_header_t*)_shared_buffer_to_vaddr(start);
    space->len = act_size;
    bitmap_set_range(bitmap, start, blocks_needed);
    vmm_tune_pages((uintptr_t)space, act_size, MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_EXEC | MMU_FLAG_PERM_READ | MMU_FLAG_NONPRIV);

    uintptr_t result_pointer = (uintptr_t)&space[1];
    umem_put_user(result_pointer, res_buffer);
    buffers[buf_id] = (uint8_t*)&space[1];
#ifdef SHARED_BUFFER_DEBUG
    log("Buffer created at %x %d", buffers[buf_id], buf_id);
#endif
    spinlock_release(&_shared_buffer_lock);
    return buf_id;
}

int shared_buffer_get(int id, uintptr_t __user* res_buffer)
{
    if (unlikely(id < 0 || SHBUF_MAX_BUFFERS <= id)) {
        return -EINVAL;
    }

    spinlock_acquire(&_shared_buffer_lock);
    if (unlikely(buffers[id] == 0)) {
        spinlock_release(&_shared_buffer_lock);
        return -EINVAL;
    }
#ifdef SHARED_BUFFER_DEBUG
    log("Buffer opened at %x %d", buffers[id], id);
#endif
    uintptr_t result_pointer = (uintptr_t)buffers[id];
    umem_put_user(result_pointer, res_buffer);
    spinlock_release(&_shared_buffer_lock);
    return 0;
}

int shared_buffer_free(int id)
{
    if (unlikely(id < 0 || SHBUF_MAX_BUFFERS <= id)) {
        return -EINVAL;
    }

    spinlock_acquire(&_shared_buffer_lock);
    if (unlikely(buffers[id] == 0)) {
        spinlock_release(&_shared_buffer_lock);
        return -EINVAL;
    }

    shared_buffer_header_t* sptr = (shared_buffer_header_t*)buffers[id];
    size_t blocks_to_delete = (sptr[-1].len + SHBUF_BLOCK_SIZE - 1) / SHBUF_BLOCK_SIZE;
    bitmap_unset_range(bitmap, _shared_buffer_to_index((uintptr_t)&sptr[-1]), blocks_to_delete);
    buffers[id] = 0;
    spinlock_release(&_shared_buffer_lock);
    return 0;
}