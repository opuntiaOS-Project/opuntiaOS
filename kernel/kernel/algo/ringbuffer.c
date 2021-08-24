/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/ringbuffer.h>

ringbuffer_t ringbuffer_create(uint32_t size)
{
    ringbuffer_t buf;
    buf.zone = zoner_new_zone(size);
    if (!buf.zone.start) {
        return buf;
    }
    buf.start = 0;
    buf.end = 0;
    return buf;
}

void ringbuffer_free(ringbuffer_t* buf)
{
    zoner_free_zone(buf->zone);
    buf->start = 0;
    buf->end = 0;
}

uint32_t ringbuffer_space_to_read(ringbuffer_t* buf)
{
    uint32_t res = buf->zone.len - buf->start + buf->end;
    if (buf->start <= buf->end) {
        res = buf->end - buf->start;
    }
    return res;
}

uint32_t ringbuffer_space_to_read_with_custom_start(ringbuffer_t* buf, uint32_t start)
{
    start %= buf->zone.len;
    uint32_t res = buf->zone.len - start + buf->end;
    if (start <= buf->end) {
        res = buf->end - start;
    }
    return res;
}

uint32_t ringbuffer_space_to_write(ringbuffer_t* buf)
{
    uint32_t res = buf->zone.len - buf->end + buf->start;
    if (buf->start > buf->end) {
        res = buf->start - buf->end;
    }
    return res;
}

uint32_t ringbuffer_read(ringbuffer_t* buf, uint8_t* holder, uint32_t siz)
{
    uint32_t i = 0;
    if (buf->start > buf->end) {
        for (; i < siz && buf->start < buf->zone.len; i++, buf->start++) {
            holder[i] = buf->zone.ptr[buf->start];
        }
        if (buf->start == buf->zone.len) {
            buf->start = 0;
        }
    }
    for (; i < siz && buf->start < buf->end; i++, buf->start++) {
        holder[i] = buf->zone.ptr[buf->start];
    }
    return i;
}

uint32_t ringbuffer_read_with_start(ringbuffer_t* buf, uint32_t start, uint8_t* holder, uint32_t siz)
{
    uint32_t i = 0;
    start %= buf->zone.len;
    if (start > buf->end) {
        for (; i < siz && start < buf->zone.len; i++, start++) {
            holder[i] = buf->zone.ptr[start];
        }
        if (start == buf->zone.len) {
            start = 0;
        }
    }
    for (; i < siz && start < buf->end; i++, start++) {
        holder[i] = buf->zone.ptr[start];
    }
    return i;
}

uint32_t ringbuffer_read_one(ringbuffer_t* buf, uint8_t* data)
{
    if (buf->start != buf->end) {
        *data = buf->zone.ptr[buf->start];
        buf->start++;
        if (buf->start == buf->zone.len) {
            buf->start = 0;
        }
        return 1;
    }
    return 0;
}

uint32_t ringbuffer_write(ringbuffer_t* buf, const uint8_t* holder, uint32_t siz)
{
    uint32_t i = 0;
    if (buf->end >= buf->start) {
        for (; i < siz && buf->end < buf->zone.len; i++, buf->end++) {
            buf->zone.ptr[buf->end] = holder[i];
        }
        if (buf->end == buf->zone.len) {
            buf->end = 0;
        }
    }
    for (; i < siz && buf->end < buf->start; i++, buf->end++) {
        buf->zone.ptr[buf->end] = holder[i];
    }
    return i;
}

uint32_t ringbuffer_write_ignore_bounds(ringbuffer_t* buf, const uint8_t* holder, uint32_t siz)
{
    uint32_t i = 0;
    for (; i < siz; i++) {
        buf->zone.ptr[buf->end++] = holder[i];
        if (buf->end == buf->zone.len) {
            buf->end = 0;
        }
    }
    return i;
}

uint32_t ringbuffer_write_one(ringbuffer_t* buf, uint8_t data)
{
    if (buf->end + 1 != buf->start) {
        buf->zone.ptr[buf->end] = data;
        buf->end++;
        if (buf->end == buf->zone.len) {
            buf->end = 0;
        }
        return 1;
    }
    return 0;
}

void ringbuffer_clear(ringbuffer_t* buf)
{
    buf->start = 0;
    buf->end = 0;
}