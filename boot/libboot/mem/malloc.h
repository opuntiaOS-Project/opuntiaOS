/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_MEM_MALLOC_H
#define _BOOT_LIBBOOT_MEM_MALLOC_H

#include <libboot/mem/mem.h>

static inline void malloc_init(void* addr, size_t size)
{
    extern void* _malloc_next_addr;
    extern void* _malloc_end_addr;

    _malloc_next_addr = addr;
    _malloc_end_addr = _malloc_next_addr + size;
}

// Current implementation is a simple linear allocator.
static void* malloc(size_t size)
{
    extern void* _malloc_next_addr;
    extern void* _malloc_end_addr;
    if (!_malloc_next_addr) {
        return NULL;
    }
    if (_malloc_next_addr >= _malloc_end_addr) {
        return NULL;
    }

    void* res = _malloc_next_addr;

    size = align_size(size, sizeof(void*));
    _malloc_next_addr += size;
    return res;
}

// Current implementation is a simple linear allocator.
static inline void* malloc_aligned(size_t size, size_t alignment)
{
    extern void* _malloc_next_addr;
    if ((size_t)_malloc_next_addr % alignment) {
        malloc(alignment - ((size_t)_malloc_next_addr % alignment));
    }
    return malloc(size);
}

static inline void free(void* ptr)
{
    UNUSED(ptr);
}

#endif // _BOOT_LIBBOOT_MEM_MALLOC_H