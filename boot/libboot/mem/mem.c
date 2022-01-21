/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libboot/mem/mem.h>

int strcmp(const char* a, const char* b)
{
    while (*a == *b && *a != 0 && *b != 0) {
        a++;
        b++;
    }

    if (*a < *b) {
        return -1;
    }
    if (*a > *b) {
        return 1;
    }
    return 0;
}

int strncmp(const char* a, const char* b, uint32_t num)
{
    while (*a == *b && *a != 0 && *b != 0 && num) {
        a++;
        b++;
        num--;
    }

    if (!num) {
        return 0;
    }

    if (*a < *b) {
        return -1;
    }
    if (*a > *b) {
        return 1;
    }
    return 0;
}

size_t strlen(const char* s)
{
    size_t i = 0;
    while (s[i] != '\0')
        ++i;
    return i;
}

void* memset(void* dest, uint8_t fll, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *((uint8_t*)dest + i) = fll;
    }
    return dest;
}

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