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

#define MB (1024 * 1024)
#define KB (1024)

#define KMALLOC_SPACE_SIZE (4 * MB)
#define KMALLOC_BLOCK_SIZE 32

void kmalloc_init(uint32_t t_start_position);
void* kmalloc(uint32_t t_size);
void* kmalloc_page_aligned();
void* kmalloc_page_aligned_only_vaddr();
void kfree(void* ptr);

#endif // __oneOS__MEM__KMALLOC_H
