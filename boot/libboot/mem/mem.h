/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_MEM_MEM_H
#define _BOOT_LIBBOOT_MEM_MEM_H

#include <libboot/types.h>

int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, uint32_t num);
size_t strlen(const char* s);
size_t strnlen(const char* s, size_t maxlen);
void* memset(void* dest, uint8_t fll, uint32_t nbytes);
void* memcpy(void* dest, const void* src, uint32_t nbytes);
void* memccpy(void* dest, const void* src, uint8_t stop, uint32_t nbytes);
void* memmove(void* dest, const void* src, uint32_t nbytes);
int memcmp(const void* src1, const void* src2, uint32_t nbytes);

static size_t align_size(size_t size, size_t align)
{
    if (size % align) {
        size += align - (size % align);
    }
    return size;
}

static inline void* copy_after_kernel(size_t kbase, void* from, size_t size, size_t* kernel_size, size_t align)
{
    void* pp = (void*)(kbase + *kernel_size);
    memcpy(pp, from, size);
    *kernel_size += align_size(size, align);
    return pp;
}

static inline void* paddr_to_vaddr(void* ptr, size_t pbase, size_t vbase)
{
    return (void*)((size_t)ptr - pbase + vbase);
}

#endif // _BOOT_LIBBOOT_MEM_MEM_H