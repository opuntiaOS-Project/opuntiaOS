/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/ringbuffer.h>
#include <mem/vmm.h>

ringbuffer_t ringbuffer_create(size_t size)
{
    ringbuffer_t buf;
    buf.zone = kmemzone_new(size);
    if (!buf.zone.start) {
        return buf;
    }

    // Calling this function will map pages for the whole range.
    vmm_ensure_writing_to_active_address_space(buf.zone.start, buf.zone.len);

    buf.start = 0;
    buf.end = 0;
    return buf;
}

void ringbuffer_free(ringbuffer_t* buf)
{
    kmemzone_free(buf->zone);
    buf->start = 0;
    buf->end = 0;
}

ssize_t ringbuffer_space_to_read_from(ringbuffer_t* buf, size_t start)
{
    start %= buf->zone.len;
    size_t res = buf->zone.len - start + buf->end;
    if (start <= buf->end) {
        res = buf->end - start;
    }
    return res;
}

ssize_t ringbuffer_space_to_read(ringbuffer_t* buf)
{
    return ringbuffer_space_to_read_from(buf, buf->start);
}

ssize_t ringbuffer_space_to_write(ringbuffer_t* buf)
{
    size_t res = buf->zone.len - buf->end + buf->start;
    if (buf->start > buf->end) {
        res = buf->start - buf->end;
    }
    return res;
}

/**
 * @brief Reads data from the ring buffer from a particular place.
 *
 * @param rbuf The target ringbuffer.
 * @param startptr The pointer to the start, should be within rbuf bounds.
 * @param buf The buffer to read to.
 * @param siz The buffer's size.
 * @return The count of read bytes.
 */
static size_t ringbuffer_read_from_impl(ringbuffer_t* rbuf, size_t* startptr, uint8_t* buf, size_t siz)
{
    size_t i = 0;
    size_t start = *startptr;

    if (start > rbuf->end) {
        size_t todo = min(siz - i, rbuf->zone.len - start);
        memcpy(&buf[i], &rbuf->zone.ptr[start], todo);
        start += todo;
        i += todo;

        if (start == rbuf->zone.len) {
            start = 0;
        }
    }
    size_t todo = min(siz - i, rbuf->end - start);
    memcpy(&buf[i], &rbuf->zone.ptr[start], todo);
    start += todo;
    i += todo;
    *startptr = start;
    return i;
}

/**
 * @brief Reads data from the ring buffer from a particular place into user buffer.
 *
 * @param rbuf The target ringbuffer.
 * @param startptr The pointer to the start, should be within rbuf bounds.
 * @param buf The buffer to read to.
 * @param siz The buffer's size.
 * @return The count of read bytes.
 */
static size_t ringbuffer_read_user_from_impl(ringbuffer_t* rbuf, size_t* startptr, uint8_t __user* buf, size_t siz)
{
    size_t i = 0;
    size_t start = *startptr;

    if (start > rbuf->end) {
        size_t todo = min(siz - i, rbuf->zone.len - start);
        umem_copy_to_user(&buf[i], &rbuf->zone.ptr[start], todo);
        start += todo;
        i += todo;

        if (start == rbuf->zone.len) {
            start = 0;
        }
    }
    size_t todo = min(siz - i, rbuf->end - start);
    umem_copy_to_user(&buf[i], &rbuf->zone.ptr[start], todo);
    start += todo;
    i += todo;
    *startptr = start;
    return i;
}

size_t ringbuffer_read_from(ringbuffer_t* rbuf, size_t start, uint8_t* buf, size_t siz)
{
    size_t mystart = start;
    mystart %= rbuf->zone.len; // normalizing start pointer.
    return ringbuffer_read_from_impl(rbuf, &mystart, buf, siz);
}

size_t ringbuffer_read(ringbuffer_t* rbuf, uint8_t* buf, size_t siz)
{
    return ringbuffer_read_from_impl(rbuf, &rbuf->start, buf, siz);
}

size_t ringbuffer_read_user_from(ringbuffer_t* rbuf, size_t start, uint8_t __user* buf, size_t siz)
{
    size_t mystart = start;
    mystart %= rbuf->zone.len; // normalizing start pointer.
    return ringbuffer_read_user_from_impl(rbuf, &mystart, buf, siz);
}

size_t ringbuffer_read_user(ringbuffer_t* rbuf, uint8_t __user* buf, size_t siz)
{
    return ringbuffer_read_user_from_impl(rbuf, &rbuf->start, buf, siz);
}

size_t ringbuffer_write(ringbuffer_t* rbuf, const uint8_t* buf, size_t siz)
{
    size_t i = 0;
    if (rbuf->end >= rbuf->start) {
        size_t todo = min(siz - i, rbuf->zone.len - rbuf->end);
        memcpy(&rbuf->zone.ptr[rbuf->end], &buf[i], todo);
        rbuf->end += todo;
        i += todo;

        if (rbuf->end == rbuf->zone.len) {
            rbuf->end = 0;
        }
    }
    size_t todo = min(siz - i, rbuf->start - rbuf->end);
    memcpy(&rbuf->zone.ptr[rbuf->end], &buf[i], todo);
    rbuf->end += todo;
    i += todo;
    return i;
}

size_t ringbuffer_write_user(ringbuffer_t* rbuf, const uint8_t __user* buf, size_t siz)
{
    size_t i = 0;
    if (rbuf->end >= rbuf->start) {
        size_t todo = min(siz - i, rbuf->zone.len - rbuf->end);
        umem_copy_from_user(&rbuf->zone.ptr[rbuf->end], &buf[i], todo);
        rbuf->end += todo;
        i += todo;

        if (rbuf->end == rbuf->zone.len) {
            rbuf->end = 0;
        }
    }
    size_t todo = min(siz - i, rbuf->start - rbuf->end);
    umem_copy_from_user(&rbuf->zone.ptr[rbuf->end], &buf[i], todo);
    rbuf->end += todo;
    i += todo;
    return i;
}

size_t ringbuffer_write_ignore_bounds(ringbuffer_t* rbuf, const uint8_t* buf, size_t siz)
{
    size_t i = 0;
    for (; i < siz; i++) {
        rbuf->zone.ptr[rbuf->end++] = buf[i];
        if (rbuf->end == rbuf->zone.len) {
            rbuf->end = 0;
        }
    }
    return i;
}

size_t ringbuffer_write_user_ignore_bounds(ringbuffer_t* rbuf, const uint8_t* __user buf, size_t siz)
{
    size_t i = 0;
    for (; i < siz; i++) {
        umem_put_user(buf[i], &rbuf->zone.ptr[rbuf->end]);
        rbuf->end++;
        if (rbuf->end == rbuf->zone.len) {
            rbuf->end = 0;
        }
    }
    return i;
}

size_t ringbuffer_write_one(ringbuffer_t* buf, uint8_t data)
{
    if (unlikely(buf->end + 1 == buf->start)) {
        return 0;
    }
    buf->zone.ptr[buf->end] = data;
    buf->end++;
    if (buf->end == buf->zone.len) {
        buf->end = 0;
    }
    return 1;
}

void ringbuffer_clear(ringbuffer_t* buf)
{
    buf->start = 0;
    buf->end = 0;
}