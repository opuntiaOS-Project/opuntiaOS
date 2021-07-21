/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/libkern.h>
#include <mem/kmalloc.h>

#ifdef __i386__
void* memset(void* dest, uint8_t fll, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *((uint8_t*)dest + i) = fll;
    }
    return dest;
}
#endif

void* memcpy(void* dest, const void* src, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *(char*)(dest + i) = *(char*)(src + i);
    }
    return dest;
}

void* memmove(void* dest, const void* src, uint32_t nbytes)
{
    if (src > dest) {
        for (int i = 0; i < nbytes; ++i) {
            *((uint8_t*)dest + i) = *((uint8_t*)src + i);
        }
    } else {
        for (int i = nbytes - 1; i >= 0; --i) {
            *((uint8_t*)dest + i) = *((uint8_t*)src + i);
        }
    }
    return dest;
}

void* memccpy(void* dest, const void* src, uint8_t stop, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *((uint8_t*)dest + i) = *((uint8_t*)src + i);
        if (*((uint8_t*)src + i) == stop) {
            return ((uint8_t*)dest + i + 1);
        }
    }
    return NULL;
}

int memcmp(const void* src1, const void* src2, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        if (*(uint8_t*)(src1 + i) < *((uint8_t*)src2 + i)) {
            return -1;
        }
        if (*(uint8_t*)(src1 + i) > *(uint8_t*)(src2 + i)) {
            return 1;
        }
    }
    return 0;
}

char* kmem_bring_to_kernel(const char* data, uint32_t size)
{
    char* kdata = kmalloc(size);
    if (kdata) {
        memcpy(kdata, data, size);
    }
    return kdata;
}

char** kmem_bring_to_kernel_ptrarr(const char** data, uint32_t size)
{
    char** res = kmalloc(size * sizeof(char*));

    for (int i = 0; i < size; i++) {
        res[i] = kmem_bring_to_kernel(data[i], strlen(data[i]) + 1);
    }

    return res;
}
