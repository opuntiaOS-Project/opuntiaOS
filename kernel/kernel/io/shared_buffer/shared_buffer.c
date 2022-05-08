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
#include <tasking/tasking.h>

// #define SHARED_BUFFER_DEBUG

#define SHBUF_SPACE_SIZE (128 * MB)
#define SHBUF_BLOCK_SIZE (4 * KB)
#define SHBUF_MAX_BUFFERS 128

struct buffer_desc {
    uint8_t* data;
    uid_t owner_uid;
};
typedef struct buffer_desc buffer_desc_t;

buffer_desc_t buffer_descs[SHBUF_MAX_BUFFERS];

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
        if (!buffer_descs[i].data) {
            return i;
        }
    }
    return -1;
}

static void _shared_buffer_init_bitmap()
{
    _shared_buffer_bitmap = _shared_buffer_zone.ptr;
    _shared_buffer_bitmap_len = (SHBUF_SPACE_SIZE / SHBUF_BLOCK_SIZE / 8);
    vmm_ensure_writing_to_active_address_space((uintptr_t)_shared_buffer_bitmap, _shared_buffer_bitmap_len);

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
    buffer_descs[buf_id].data = (uint8_t*)result_pointer;
    buffer_descs[buf_id].owner_uid = RUNNING_THREAD->process->pid;
    umem_put_user(result_pointer, res_buffer);

#ifdef SHARED_BUFFER_DEBUG
    log("Buffer created at %p %d", buffer_descs[buf_id].data, buf_id);
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
    if (unlikely(!buffer_descs[id].data)) {
        spinlock_release(&_shared_buffer_lock);
        return -EINVAL;
    }

    if (!proc_is_su(RUNNING_THREAD->process) && RUNNING_THREAD->process->pid != buffer_descs[id].owner_uid) {
        spinlock_release(&_shared_buffer_lock);
        return -EPERM;
    }

#ifdef SHARED_BUFFER_DEBUG
    log("Buffer opened at %x %d", buffer_descs[id].data, id);
#endif
    uintptr_t result_pointer = (uintptr_t)buffer_descs[id].data;
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
    if (unlikely(!buffer_descs[id].data)) {
        spinlock_release(&_shared_buffer_lock);
        return -EINVAL;
    }

    if (!proc_is_su(RUNNING_THREAD->process) && RUNNING_THREAD->process->pid != buffer_descs[id].owner_uid) {
        spinlock_release(&_shared_buffer_lock);
        return -EPERM;
    }

    shared_buffer_header_t* sptr = (shared_buffer_header_t*)buffer_descs[id].data;
    size_t blocks_to_delete = (sptr[-1].len + SHBUF_BLOCK_SIZE - 1) / SHBUF_BLOCK_SIZE;
    bitmap_unset_range(bitmap, _shared_buffer_to_index((uintptr_t)&sptr[-1]), blocks_to_delete);
    buffer_descs[id].data = NULL;
    spinlock_release(&_shared_buffer_lock);
    return 0;
}