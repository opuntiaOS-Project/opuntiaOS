/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/libkern.h>
#include <mem/kmalloc.h>

#ifdef __i386__
void* memset(void* dest, uint8_t fll, size_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *((uint8_t*)dest + i) = fll;
    }
    return dest;
}
#endif

void* memcpy(void* dest, const void* src, size_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *(char*)(dest + i) = *(char*)(src + i);
    }
    return dest;
}

void* memmove(void* dest, const void* src, size_t nbytes)
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

void* memccpy(void* dest, const void* src, uint8_t stop, size_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *((uint8_t*)dest + i) = *((uint8_t*)src + i);
        if (*((uint8_t*)src + i) == stop) {
            return ((uint8_t*)dest + i + 1);
        }
    }
    return NULL;
}

int memcmp(const void* src1, const void* src2, size_t nbytes)
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
