/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *  + Contributed by bellrise <bellrise.dev@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef _LIBC_STRING_H
#define _LIBC_STRING_H

#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

/* Set 'nbytes' to 'fill' starting from 'dest'. */
void* memset(void* dest, int fill, size_t nbytes);

/* Move 'nbytes' from 'src' to 'dest' */
void* memmove(void* dest, const void* __restrict src, size_t nbytes);

/* Copy 'nbytes' from 'src' to 'dest'. See the comment in the source file
   about optimization and restricting pointers. */
void* memcpy(void* __restrict dest, const void* __restrict src, size_t nbytes);

/* Copy 'nbytes' from 'src' to 'dest', stopping if the current byte matches
   'stop'. Note that the stop byte also gets copied over. */
void* memccpy(void* dest, const void* __restrict src, int stop, size_t nbytes);

/* Compare 'nbytes' from 'src1' and 'src2'. Return 0 if all the bytes match,
   otherwise return the difference. */
int memcmp(const void* src1, const void* src2, size_t nbytes);

/* Calculate the string length starting from 'str'. */
size_t strlen(const char* str);

/* Copy 'src' into 'dest' until it finds a null byte in the source string.
   Note that this is dangerous because it writes memory no matter the size
   the 'dest' buffer is. */
char* strcpy(char* dest, const char* src);
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, size_t nbytes);

/* Copy 'src' into 'dest' until it finds a null byte or reaches the 'nbytes'
   limit provided by the user. This is the recommended way of copying strings,
   instead of using regular strcpy. Note that this will fill the 'dest' buffer
   with null bytes if the amount of copied bytes is lower than 'nbytes'. */
char* strncpy(char* dest, const char* src, size_t nbytes);

/* Returns a pointer to the first occurrence of character in the C string str. */
char* strchr(const char* s, int c);

__END_DECLS

#endif // _LIBC_STRING_H
