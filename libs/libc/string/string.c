/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 *
 * Modified by: bellrise
 */
#include <string.h>

#ifdef __i386__
void* memset(void* dest, int fill, size_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        ((uint8_t*)dest)[i] = fill;
    }
    return dest;
}
#endif //__i386__

void* memmove(void* dest, const void* src, size_t nbytes)
{
    if (src > dest) {
        for (int i = 0; i < nbytes; ++i)
            ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    } else {
        for (int i = nbytes - 1; i >= 0; --i)
            ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];
    }
    return dest;
}

/* This optimized version of memcpy uses 32 bit chunks to copy over data on
   the 32 bit architecture this library is built for. If this function gets
   used on a 64 bit arch, be sure to use 8 byte chunks so each chunk fits
   in a single register. The important part is this should be compiled with
   atleast -O1 or -Os, because -O0 just makes this function too big for what
   it does.

   GCC does a better job at optimizing this if the pointers are restricted,
   making the copying part have less instructions. Clang on the other hand
   does not really change anything if the pointers are restricted or not. */
void* memcpy(void* __restrict dest, const void* __restrict src, size_t nbytes)
{
    size_t chunks, rest, i;

    rest = nbytes % 4;
    chunks = (nbytes - rest) >> 2;

    if (!chunks)
        goto skip_chunks;

    for (i = 0; i < chunks; i++)
        ((uint32_t *) dest)[i] = ((uint32_t*)src)[i];

skip_chunks:

    /* Multiplying chunks by 4 will give us the offset of the 'rest' bytes,
       which were not copied over along with the 4 byte chunks. */
    chunks <<= 2;

    for (i = 0; i < rest; i++)
        ((uint8_t*)dest)[chunks + i] = ((uint8_t*)src)[chunks + i];

    return dest;
}

void* memccpy(void* dest, const void* src, int stop, size_t nbytes)
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
        if (*((uint8_t*)src1 + i) < *((uint8_t*)src2 + i)) {
            return -1;
        }
        if (*((uint8_t*)src1 + i) > *((uint8_t*)src2 + i)) {
            return 1;
        }
    }
    return 0;
}

size_t strlen(const char* str)
{
    size_t i = 0;
    while (str[i])
        ++i;
    return i;
}

char* strcpy(char* dest, const char* src)
{
    size_t i;
    for (i = 0; src[i] != 0; i++) {
        dest[i] = src[i];
    }
    dest[i] = '\0';
    return dest;
}

char* strncpy(char* dest, const char* src, size_t nbytes)
{
    size_t i;

    for (i = 0; i < nbytes && src[i] != 0; i++)
        dest[i] = src[i];

    /* Fill the rest with null bytes */
    for (; i < nbytes; i++)
        dest[i] = 0;

    return dest;
}
