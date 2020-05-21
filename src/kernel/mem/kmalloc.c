/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <drivers/display.h>
#include <mem/kmalloc.h>

struct kmalloc_header {
    uint32_t len;
};
typedef struct kmalloc_header kmalloc_header_t;

static void* _kmalloc_data_start;
static uint32_t _kmalloc_bitmap_len = 0;
static uint8_t* kmalloc_bitmap;

static void _kmalloc_init_bitmap()
{
    _kmalloc_bitmap_len = (KMALLOC_SPACE_SIZE / KMALLOC_BLOCK_SIZE / 8);
    _kmalloc_data_start = (void*)(kmalloc_bitmap + _kmalloc_bitmap_len);
    memset(kmalloc_bitmap, 0, _kmalloc_bitmap_len);
}

void kmalloc_init(uint32_t start_position)
{
    kmalloc_bitmap = (uint8_t*)start_position;
    _kmalloc_init_bitmap();
}

// TODO: can be speeded up with tzcnt
static int kmalloc_find_space(int req)
{
    int taken = 0;
    int start = 0;
    for (int i = 0; i < _kmalloc_bitmap_len; i++) {
        if (kmalloc_bitmap[i] == 0xff) {
            continue;
        }
        for (int j = 0; j < 8; j++) {
            if ((kmalloc_bitmap[i] >> j) & 1) {
                taken = 0;
            } else {
                if (taken == 0) {
                    start = i * 8 + j;
                }
                taken++;
                if (taken == req) {
                    return start;
                }
            }
        }
    }
    return -1;
}

static void kmalloc_take(int start, int len)
{
    for (int i = start; i < start + len; i++) {
        int block = i / 8;
        int offset = i % 8;
        kmalloc_bitmap[block] |= (1 << offset);
    }
}

static void kmalloc_free(int start, int len)
{
    for (int i = start; i < start + len; i++) {
        int block = i / 8;
        int offset = i % 8;
        kmalloc_bitmap[block] &= ~((1 << offset));
    }
}

static inline uint32_t kmalloc_to_vaddr(int start)
{
    uint32_t vaddr = (uint32_t)_kmalloc_data_start + start * KMALLOC_BLOCK_SIZE;
    return (uint32_t)_kmalloc_data_start + start * KMALLOC_BLOCK_SIZE;
}

static inline int kmalloc_to_index(uint32_t vaddr)
{
    return (vaddr - (uint32_t)_kmalloc_data_start) / KMALLOC_BLOCK_SIZE;
}

void* kmalloc(uint32_t size)
{
    int act_size = size + sizeof(kmalloc_header_t);

    int blocks_needed = (act_size + KMALLOC_BLOCK_SIZE - 1) / KMALLOC_BLOCK_SIZE;

    int start = kmalloc_find_space(blocks_needed);
    if (start < 0) {
        return 0;
    }

    kmalloc_header_t* space = (kmalloc_header_t*)kmalloc_to_vaddr(start);
    space->len = act_size;
    kmalloc_take(start, blocks_needed);

    return (void*)&space[1];
}

void* kmalloc_aligned(uint32_t size, uint32_t alignment)
{
    void* ptr = kmalloc(size + alignment + sizeof(void*));
    uint32_t max_addr = (uint32_t)ptr + alignment;
    void* aligned_ptr = (void*)(max_addr - (max_addr % alignment));
    ((void**)aligned_ptr)[-1] = ptr;
    return aligned_ptr;
}

void* kmalloc_page_aligned()
{
    return kmalloc_aligned(VMM_PAGE_SIZE, VMM_PAGE_SIZE);
}

void kfree(void* ptr)
{
    kmalloc_header_t* sptr = (kmalloc_header_t*)ptr;
    int blocks_to_delete = (sptr[-1].len + KMALLOC_BLOCK_SIZE - 1) / KMALLOC_BLOCK_SIZE;
    kmalloc_free(kmalloc_to_index((uint32_t)&sptr[-1]), blocks_to_delete);
}

void kfree_aligned(void* ptr)
{
    kfree(((void**)ptr)[-1]);
}
