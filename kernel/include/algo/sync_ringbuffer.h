/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_ALGO_SYNC_RINGBUFFER_H
#define _KERNEL_ALGO_SYNC_RINGBUFFER_H

#include <algo/ringbuffer.h>
#include <libkern/libkern.h>
#include <libkern/lock.h>
#include <mem/vmm/zoner.h>

struct __sync_ringbuffer {
    ringbuffer_t ringbuffer;
    lock_t lock;
};
typedef struct __sync_ringbuffer sync_ringbuffer_t;

static ALWAYS_INLINE sync_ringbuffer_t sync_ringbuffer_create(uint32_t size)
{
    sync_ringbuffer_t res;
    res.ringbuffer = ringbuffer_create(size);
    lock_init(&res.lock);
    return res;
}

#define sync_ringbuffer_create_std() sync_ringbuffer_create(RINGBUFFER_STD_SIZE)
static ALWAYS_INLINE void sync_ringbuffer_free(sync_ringbuffer_t* buf)
{
    lock_acquire(&buf->lock);
    ringbuffer_free(&buf->ringbuffer);
    lock_release(&buf->lock);
}

static ALWAYS_INLINE uint32_t sync_ringbuffer_space_to_read(sync_ringbuffer_t* buf)
{
    lock_acquire(&buf->lock);
    uint32_t res = ringbuffer_space_to_read(&buf->ringbuffer);
    lock_release(&buf->lock);
    return res;
}

static ALWAYS_INLINE uint32_t sync_ringbuffer_space_to_read_with_custom_start(sync_ringbuffer_t* buf, uint32_t start)
{
    lock_acquire(&buf->lock);
    uint32_t res = ringbuffer_space_to_read_with_custom_start(&buf->ringbuffer, start);
    lock_release(&buf->lock);
    return res;
}
static ALWAYS_INLINE uint32_t sync_ringbuffer_space_to_write(sync_ringbuffer_t* buf)
{
    lock_acquire(&buf->lock);
    uint32_t res = ringbuffer_space_to_write(&buf->ringbuffer);
    lock_release(&buf->lock);
    return res;
}
static ALWAYS_INLINE uint32_t sync_ringbuffer_read(sync_ringbuffer_t* buf, uint8_t* v, uint32_t a)
{
    lock_acquire(&buf->lock);
    uint32_t res = ringbuffer_read(&buf->ringbuffer, v, a);
    lock_release(&buf->lock);
    return res;
}
static ALWAYS_INLINE uint32_t sync_ringbuffer_read_with_start(sync_ringbuffer_t* buf, uint32_t start, uint8_t* holder, uint32_t siz)
{
    lock_acquire(&buf->lock);
    uint32_t res = ringbuffer_read_with_start(&buf->ringbuffer, start, holder, siz);
    lock_release(&buf->lock);
    return res;
}
static ALWAYS_INLINE uint32_t sync_ringbuffer_write(sync_ringbuffer_t* buf, const uint8_t* v, uint32_t a)
{
    lock_acquire(&buf->lock);
    uint32_t res = ringbuffer_write(&buf->ringbuffer, v, a);
    lock_release(&buf->lock);
    return res;
}
static ALWAYS_INLINE uint32_t sync_ringbuffer_write_ignore_bounds(sync_ringbuffer_t* buf, const uint8_t* holder, uint32_t siz)
{
    lock_acquire(&buf->lock);
    uint32_t res = ringbuffer_write_ignore_bounds(&buf->ringbuffer, holder, siz);
    lock_release(&buf->lock);
    return res;
}
static ALWAYS_INLINE uint32_t sync_ringbuffer_read_one(sync_ringbuffer_t* buf, uint8_t* data)
{
    lock_acquire(&buf->lock);
    uint32_t res = ringbuffer_read_one(&buf->ringbuffer, data);
    lock_release(&buf->lock);
    return res;
}
static ALWAYS_INLINE uint32_t sync_ringbuffer_write_one(sync_ringbuffer_t* buf, uint8_t data)
{
    lock_acquire(&buf->lock);
    uint32_t res = ringbuffer_write_one(&buf->ringbuffer, data);
    lock_release(&buf->lock);
    return res;
}

static ALWAYS_INLINE void sync_ringbuffer_clear(sync_ringbuffer_t* buf)
{
    lock_acquire(&buf->lock);
    ringbuffer_clear(&buf->ringbuffer);
    lock_release(&buf->lock);
}

#endif //_KERNEL_ALGO_SYNC_RINGBUFFER_H