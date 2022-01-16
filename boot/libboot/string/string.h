/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_STRING_STRING_H
#define _BOOT_LIBBOOT_STRING_STRING_H

#include <libboot/types.h>

int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, uint32_t num);
size_t strlen(const char* s);
void* memset(void* dest, uint8_t fll, uint32_t nbytes);
void* memcpy(void* dest, const void* src, uint32_t nbytes);
void* memccpy(void* dest, const void* src, uint8_t stop, uint32_t nbytes);
void* memmove(void* dest, const void* src, uint32_t nbytes);
int memcmp(const void* src1, const void* src2, uint32_t nbytes);

#endif // _BOOT_LIBBOOT_STRING_STRING_H