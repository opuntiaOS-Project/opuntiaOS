/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libboot/log/log.h>
#include <libboot/mem/alloc.h>

void* _malloc_next_addr = NULL;
void* _malloc_end_addr = NULL;

void* _palloc_start_addr = NULL;
void* _palloc_next_addr = NULL;
void* _palloc_end_addr = NULL;

int malloc_init(void* addr, size_t size)
{
    _malloc_next_addr = addr;
    _malloc_end_addr = _malloc_next_addr + size;
    return 0;
}

// Current implementation is a simple linear allocator.
void* malloc(size_t size)
{
    if (!_malloc_next_addr) {
        return NULL;
    }
    if (_malloc_next_addr >= _malloc_end_addr) {
        return NULL;
    }

    void* res = _malloc_next_addr;

    size = ROUND_CEIL(size, 0x10);
    _malloc_next_addr += size;
    return res;
}

// Current implementation is a simple linear allocator.
void* malloc_aligned(size_t size, size_t alignment)
{
    if ((size_t)_malloc_next_addr % alignment) {
        malloc(alignment - ((size_t)_malloc_next_addr % alignment));
    }
    return malloc(size);
}

void free(void* ptr)
{
    UNUSED(ptr);
}

// Preserve alloc

int palloc_init(size_t size, size_t alignment)
{
    _palloc_start_addr = _palloc_next_addr = malloc_aligned(size, alignment);
    if (!_palloc_next_addr) {
        return -1;
    }

    _palloc_end_addr = (void*)((uintptr_t)_palloc_next_addr + size);
    return 0;
}

void* palloc(size_t size)
{
    if (!_palloc_next_addr) {
        log("palloc not init");
        while (1) { };
    }
    if (_palloc_next_addr >= _palloc_end_addr) {
        log("palloc out of mem");
        while (1) { };
    }

    void* res = _palloc_next_addr;

    size = ROUND_CEIL(size, 0x10);
    _palloc_next_addr += size;
    return res;
}

void* palloc_aligned(size_t size, size_t alignment)
{
    if ((size_t)_palloc_next_addr % alignment) {
        palloc(alignment - ((size_t)_palloc_next_addr % alignment));
    }
    return palloc(size);
}

size_t palloc_total_size()
{
    return (size_t)_palloc_end_addr - (size_t)_palloc_start_addr;
}

size_t palloc_used_size()
{
    return (size_t)_palloc_next_addr - (size_t)_palloc_start_addr;
}
