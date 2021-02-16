/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__ALGO__RINGBUFFER_H
#define __oneOS__ALGO__RINGBUFFER_H

#include <mem/vmm/zoner.h>
#include <types.h>

struct __ringbuffer {
    zone_t zone;
    uint32_t start;
    uint32_t end;
};
typedef struct __ringbuffer ringbuffer_t;

ringbuffer_t ringbuffer_create(uint32_t size);
ringbuffer_t ringbuffer_create_std();
void ringbuffer_free(ringbuffer_t* buf);

uint32_t ringbuffer_space_to_read(ringbuffer_t* buf);
uint32_t ringbuffer_space_to_read_with_custom_start(ringbuffer_t* buf, uint32_t start);
uint32_t ringbuffer_space_to_write(ringbuffer_t* buf);
uint32_t ringbuffer_read(ringbuffer_t* buf, uint8_t*, uint32_t);
uint32_t ringbuffer_read_with_start(ringbuffer_t* buf, uint32_t start, uint8_t* holder, uint32_t siz);
uint32_t ringbuffer_write(ringbuffer_t* buf, const uint8_t*, uint32_t);
uint32_t ringbuffer_write_ignore_bounds(ringbuffer_t* buf, const uint8_t* holder, uint32_t siz);
uint32_t ringbuffer_read_one(ringbuffer_t* buf, uint8_t* data);
uint32_t ringbuffer_write_one(ringbuffer_t* buf, uint8_t data);
void ringbuffer_clear(ringbuffer_t* buf);

#endif //__oneOS__ALGO__RINGBUFFER_H