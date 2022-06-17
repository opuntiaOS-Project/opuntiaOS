/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/libkern.h>

#ifndef __arm__
// TODO: Implement tuned memset for each arch.
void* memset(void* dest, uint8_t fll, size_t nbytes)
{
    if (nbytes < 16) {
        for (int i = 0; i < nbytes; i++) {
            *((uint8_t*)dest + i) = fll;
        }
        return dest;
    }

    uintptr_t destaddr = (uintptr_t)dest;
    size_t aligment = (8 - (destaddr % 8)) % 8;
    for (int i = 0; i < aligment; i++) {
        *((uint8_t*)dest + i) = fll;
    }

    nbytes -= aligment;
    uintptr_t destaddr_aligned = destaddr + aligment;
    size_t words = nbytes / 8;

    uint64_t fll16 = (fll << 8) | fll;
    uint64_t fll32 = (fll16 << 16) | fll16;
    uint64_t fll64 = (fll32 << 32) | fll32;
    uint64_t* destu64 = (uint64_t*)destaddr_aligned;
    for (size_t i = 0; i < words; i++) {
        destu64[i] = fll64;
    }

    uintptr_t destaddr_tail = (uintptr_t)&destu64[words];
    size_t rembytes = nbytes - words * 8;
    for (size_t i = 0; i < rembytes; i++) {
        *((uint8_t*)destaddr_tail + i) = fll;
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
