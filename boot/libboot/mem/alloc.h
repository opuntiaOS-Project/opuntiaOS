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

int malloc_init(void* addr, size_t size);
void* malloc(size_t size);
void* malloc_aligned(size_t size, size_t alignment);
void free(void* ptr);

int palloc_init(size_t size, size_t alignment);
void* palloc(size_t size);
void* palloc_aligned(size_t size, size_t alignment);
size_t palloc_used_size();
size_t palloc_total_size();

#endif // _BOOT_LIBBOOT_MEM_MALLOC_H