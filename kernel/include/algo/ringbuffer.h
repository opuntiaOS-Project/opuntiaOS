/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
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
#define ringbuffer_create_std() ringbuffer_create(RINGBUFFER_STD_SIZE)
void ringbuffer_free(ringbuffer_t* buf);

size_t ringbuffer_space_to_read(ringbuffer_t* buf);
size_t ringbuffer_space_to_read_with_custom_start(ringbuffer_t* buf, size_t start);
size_t ringbuffer_space_to_write(ringbuffer_t* buf);
size_t ringbuffer_read(ringbuffer_t* buf, uint8_t*, size_t);
size_t ringbuffer_read_with_start(ringbuffer_t* buf, size_t start, uint8_t* holder, size_t siz);
size_t ringbuffer_write(ringbuffer_t* buf, const uint8_t*, size_t);
size_t ringbuffer_write_ignore_bounds(ringbuffer_t* buf, const uint8_t* holder, size_t siz);
size_t ringbuffer_read_one(ringbuffer_t* buf, uint8_t* data);
size_t ringbuffer_write_one(ringbuffer_t* buf, uint8_t data);
void ringbuffer_clear(ringbuffer_t* buf);

#endif //_KERNEL_ALGO_RINGBUFFER_H