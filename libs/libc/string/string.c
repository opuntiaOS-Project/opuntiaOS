/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *  + Contributed by bellrise <bellrise.dev@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <stdbool.h>
#include <string.h>

#ifndef __arm__
void* memset(void* dest, int fill, size_t nbytes)
{
    for (int i = 0; i < nbytes; ++i)
        ((uint8_t*)dest)[i] = fill;

    return dest;
}
#endif //__arm__

void* memmove(void* dest, const void* src, size_t nbytes)
{
    if (src > dest) {
        memcpy(dest, src, nbytes);
        return dest;
    }

    for (int i = nbytes - 1; i >= 0; --i)
        ((uint8_t*)dest)[i] = ((uint8_t*)src)[i];

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
        ((uint32_t*)dest)[i] = ((uint32_t*)src)[i];

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
    for (int i = 0; i < nbytes; i++) {
        *((uint8_t*)dest + i) = *((uint8_t*)src + i);

        if (*((uint8_t*)src + i) == stop)
            return ((uint8_t*)dest + i + 1);
    }
    return NULL;
}

int memcmp(const void* src1, const void* src2, size_t nbytes)
{
    const uint8_t* first = src1;
    const uint8_t* second = src2;

    for (int i = 0; i < nbytes; i++) {
        /* Return the difference if the byte does not match. */
        if (first[i] != second[i])
            return (int)first[i] - (int)second[i];
    }

    return 0;
}

void* memchr(const void* ptr, int c, size_t size)
{
    char ch = c;
    const char* cptr = (const char*)ptr;
    for (size_t i = 0; i < size; i++) {
        if (cptr[i] == ch) {
            return (char*)(cptr + i);
        }
    }
    return NULL;
}

int strcmp(const char* a, const char* b)
{
    while (*a == *b && *a != '\0' && *b != '\0') {
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

int strncmp(const char* a, const char* b, size_t num)
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
    for (i = 0; src[i] != 0; i++)
        dest[i] = src[i];

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

char* strchr(const char* s, int c)
{
    for (;; s++) {
        if (*s == c) {
            return (char*)s;
        }
        if (!(*s)) {
            return NULL;
        }
    }
}

char* strtok_r(char* str, const char* delim, char** saveptr)
{
    if (!str) {
        if (!saveptr) {
            return NULL;
        }
        str = *saveptr;
    }

    size_t start = 0;
    size_t end = 0;
    size_t n = strlen(str);
    size_t m = strlen(delim);
    bool ok = false;

    for (size_t i = 0; i < n; i++) {
        ok = false;
        for (size_t j = 0; j < m; j++) {
            if (str[i] == delim[j]) {
                if (end - start == 0) {
                    start++;
                    break;
                }

                ok = true;
            }
        }

        if (ok) {
            break;
        }
        end++;
    }

    if (str[start] == '\0') {
        return NULL;
    }

    if (end == 0) {
        *saveptr = NULL;
        return &str[start];
    }

    if (str[end] == '\0') {
        *saveptr = &str[end];
    } else {
        *saveptr = &str[end + 1];
    }

    str[end] = '\0';
    return &str[start];
}

char* strtok(char* str, const char* delim)
{
    static char* saveptr;
    return strtok_r(str, delim, &saveptr);
}

char* strstr(const char* haystack, const char* needle)
{
    size_t n = strlen(needle);

    while (*haystack) {
        if (!memcmp(haystack, needle, n)) {
            return (char*)haystack;
        }
        haystack++;
    }
    return NULL;
}

char* strcat(char* dest, const char* src)
{
    size_t dest_len = strlen(dest);
    size_t i = 0;

    while (src[i] != '\0') {
        dest[dest_len + i] = src[i];
        i++;
    }
    dest[dest_len + i] = '\0';
    return dest;
}

char* strrchr(const char* str, int ch)
{
    char c;
    char* last = NULL;

    while ((c = *str)) {
        if (c == ch) {
            last = (char*)(str);
        }
        str++;
    }
    return last;
}
