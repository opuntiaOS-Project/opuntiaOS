/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_KMALLOC_H
#define _KERNEL_MEM_KMALLOC_H

#include <libkern/types.h>
#include <mem/vmm/vmm.h>

#define KMALLOC_SPACE_SIZE (4 * MB)
#define KMALLOC_BLOCK_SIZE 32

void kmalloc_init();
void* kmalloc(uint32_t size);
void* kmalloc_aligned(uint32_t size, uint32_t alignment);
void* kmalloc_page_aligned();
void kfree(void* ptr);
void kfree_aligned(void* ptr);
void* krealloc(void* ptr, uint32_t size);

#endif // _KERNEL_MEM_KMALLOC_H
