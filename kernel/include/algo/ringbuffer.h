/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_ALGO_RINGBUFFER_H
#define _KERNEL_ALGO_RINGBUFFER_H

#include <libkern/libkern.h>
#include <libkern/lock.h>
#include <mem/kmemzone.h>

#define RINGBUFFER_STD_SIZE (16 * KB)

struct __ringbuffer {
    kmemzone_t zone;
    size_t start;
    size_t end;
};
typedef struct __ringbuffer ringbuffer_t;

ringbuffer_t ringbuffer_create(size_t size);
static ALWAYS_INLINE ringbuffer_t ringbuffer_create_std() { return ringbuffer_create(RINGBUFFER_STD_SIZE); }
void ringbuffer_free(ringbuffer_t* rbuf);

ssize_t ringbuffer_space_to_read_from(ringbuffer_t* rbuf, size_t start);
ssize_t ringbuffer_space_to_read(ringbuffer_t* rbuf);
ssize_t ringbuffer_space_to_write(ringbuffer_t* rbuf);

size_t ringbuffer_read_from(ringbuffer_t* rbuf, size_t ustart, uint8_t __user* buf, size_t siz);
size_t ringbuffer_read_user_from(ringbuffer_t* rbuf, size_t ustart, uint8_t __user* buf, size_t siz);
size_t ringbuffer_read(ringbuffer_t* rbuf, uint8_t*, size_t);
size_t ringbuffer_read_user(ringbuffer_t* rbuf, uint8_t __user* buf, size_t siz);

size_t ringbuffer_write(ringbuffer_t* rbuf, const uint8_t*, size_t);
size_t ringbuffer_write_user(ringbuffer_t* rbuf, const uint8_t __user* buf, size_t siz);
size_t ringbuffer_write_ignore_bounds(ringbuffer_t* rbuf, const uint8_t* buf, size_t siz);
size_t ringbuffer_write_user_ignore_bounds(ringbuffer_t* rbuf, const uint8_t* __user buf, size_t siz);

size_t ringbuffer_read_one(ringbuffer_t* rbuf, uint8_t* data);
size_t ringbuffer_write_one(ringbuffer_t* rbuf, uint8_t data);
void ringbuffer_clear(ringbuffer_t* rbuf);

#endif //_KERNEL_ALGO_RINGBUFFER_H