/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__MEM__KMALLOC_H
#define __oneOS__MEM__KMALLOC_H

#include <types.h>
#include <mem/vmm/vmm.h>

#define KMALLOC_SPACE_SIZE (4 * MB)
#define KMALLOC_BLOCK_SIZE 32

void kmalloc_init();
void* kmalloc(uint32_t size);
void* kmalloc_page_aligned();
void kfree(void* ptr);
void kfree_aligned(void* ptr);

#endif // __oneOS__MEM__KMALLOC_H
