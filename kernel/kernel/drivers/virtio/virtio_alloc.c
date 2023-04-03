

/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/devtree.h>
#include <drivers/virtio/virtio.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <tasking/cpu.h>
#include <tasking/sched.h>
#include <time/time_manager.h>

#define DEBUG_VIRTIO

struct virtio_alloc_header {
    size_t len;
};
typedef struct virtio_alloc_header virtio_alloc_header_t;

#define VIRTIO_ALLOC_SPACE_SIZE (4 * MB)
#define VIRTIO_ALLOC_BLOCK_SIZE 32

static spinlock_t _virtio_alloc_lock;
static kmemzone_t _virtio_alloc_zone;
static void* _virtio_alloc_paddr;
static size_t _virtio_alloc_bitmap_len = 0;
static uint8_t* _virtio_alloc_bitmap;
static bitmap_t bitmap;

static inline uintptr_t virtio_alloc_to_vaddr(int start)
{
    uintptr_t vaddr = (uintptr_t)_virtio_alloc_zone.start + start * VIRTIO_ALLOC_BLOCK_SIZE;
    return (uintptr_t)_virtio_alloc_zone.start + start * VIRTIO_ALLOC_BLOCK_SIZE;
}

static inline uintptr_t vaddr_to_paddr(uintptr_t vaddr)
{
    return vaddr - (uintptr_t)_virtio_alloc_zone.start + (uintptr_t)_virtio_alloc_paddr;
}

static inline uintptr_t paddr_to_vaddr(uintptr_t paddr)
{
    return paddr - (uintptr_t)_virtio_alloc_paddr + (uintptr_t)_virtio_alloc_zone.start;
}

static inline int virtio_alloc_to_index(uintptr_t vaddr)
{
    return (vaddr - (uintptr_t)_virtio_alloc_zone.start) / VIRTIO_ALLOC_BLOCK_SIZE;
}

static void _virtio_alloc_init_bitmap()
{
    _virtio_alloc_bitmap = (uint8_t*)_virtio_alloc_zone.start;
    _virtio_alloc_bitmap_len = (VIRTIO_ALLOC_SPACE_SIZE / VIRTIO_ALLOC_BLOCK_SIZE / 8);

    size_t pages_cnt = (VIRTIO_ALLOC_SPACE_SIZE + VMM_PAGE_SIZE - 1) / VMM_PAGE_SIZE;
    vmm_map_pages(_virtio_alloc_zone.start, (uintptr_t)_virtio_alloc_paddr, pages_cnt, MMU_FLAG_DEVICE);

    bitmap = bitmap_wrap(_virtio_alloc_bitmap, _virtio_alloc_bitmap_len);
    memset(_virtio_alloc_bitmap, 0, _virtio_alloc_bitmap_len);

    /* Setting bitmap as a busy region. */
    int blocks_needed = (_virtio_alloc_bitmap_len + VIRTIO_ALLOC_BLOCK_SIZE - 1) / VIRTIO_ALLOC_BLOCK_SIZE;
    bitmap_set_range(bitmap, virtio_alloc_to_index((uintptr_t)_virtio_alloc_bitmap), blocks_needed);
}

int virtio_alloc_init()
{
    spinlock_init(&_virtio_alloc_lock);
    _virtio_alloc_zone = kmemzone_new(VIRTIO_ALLOC_SPACE_SIZE);
    _virtio_alloc_paddr = pmm_alloc(VIRTIO_ALLOC_SPACE_SIZE);
    _virtio_alloc_init_bitmap();

    return 0;
}

int virtio_alloc(size_t size, virtio_alloc_result_t* result)
{
    spinlock_acquire(&_virtio_alloc_lock);
    size_t alloc_size = size;

    int act_size = alloc_size + sizeof(virtio_alloc_header_t);

    int blocks_needed = (act_size + VIRTIO_ALLOC_BLOCK_SIZE - 1) / VIRTIO_ALLOC_BLOCK_SIZE;

    int start = bitmap_find_space(bitmap, blocks_needed);
    if (start < 0) {
        log_error("NO SPACE AT VIRTIO_ALLOC");
        system_stop();
    }

    virtio_alloc_header_t* space = (virtio_alloc_header_t*)virtio_alloc_to_vaddr(start);
    bitmap_set_range(bitmap, start, blocks_needed);
    spinlock_release(&_virtio_alloc_lock);

    space->len = act_size;
    void* result_vaddr = (void*)&space[1];

    result->req_vaddr = result_vaddr;
    result->req_paddr = (void*)vaddr_to_paddr((uintptr_t)result_vaddr);

    return 0;
}

void virtio_free_paddr(void* paddr_ptr)
{
    if (!paddr_ptr) {
        DEBUG_ASSERT("NULL at kfree");
        return;
    }

    void* ptr = (void*)paddr_to_vaddr((uintptr_t)paddr_ptr);
    virtio_alloc_header_t* sptr = (virtio_alloc_header_t*)ptr;

    int blocks_to_delete = (sptr[-1].len + VIRTIO_ALLOC_BLOCK_SIZE - 1) / VIRTIO_ALLOC_BLOCK_SIZE;
    spinlock_acquire(&_virtio_alloc_lock);
    bitmap_unset_range(bitmap, virtio_alloc_to_index((size_t)&sptr[-1]), blocks_to_delete);
    spinlock_release(&_virtio_alloc_lock);
}

int virtio_alloc_buffer(size_t size, virtio_buffer_desc_t* result)
{
    size_t alloc_size = ROUND_CEIL(size, VMM_PAGE_SIZE);
    size_t pages_cnt = (alloc_size + VMM_PAGE_SIZE - 1) / VMM_PAGE_SIZE;

    result->kzone = kmemzone_new(alloc_size);
    char* paddr_zone = pmm_alloc(alloc_size);
    vmm_map_pages(result->kzone.start, (uintptr_t)paddr_zone, pages_cnt, MMU_FLAG_DEVICE);

    result->vaddr = (uintptr_t)result->kzone.ptr;
    result->paddr = (uintptr_t)paddr_zone;
    return 0;
}

int virtio_alloc_queue(virtio_queue_desc_t* result)
{
    size_t page_one_size = ROUND_CEIL(sizeof(virtio_desc_array_t) + sizeof(virtio_avail_t), VMM_PAGE_SIZE);
    size_t page_two_size = ROUND_CEIL(sizeof(virtio_used_t), VMM_PAGE_SIZE);

    size_t queue_alloc_size = page_one_size + page_two_size;
    size_t queue_pages_cnt = (queue_alloc_size + VMM_PAGE_SIZE - 1) / VMM_PAGE_SIZE;

    kmemzone_t queue_zone = kmemzone_new(queue_alloc_size);
    char* q_paddr_zone = pmm_alloc(queue_alloc_size);
    vmm_map_pages(queue_zone.start, (uintptr_t)q_paddr_zone, queue_pages_cnt, MMU_FLAG_DEVICE);
    void* queue_ptr = queue_zone.ptr;

    result->descs = (virtio_desc_array_t*)queue_zone.ptr;
    result->avail = (virtio_avail_t*)(queue_zone.start + sizeof(virtio_desc_array_t));
    result->used = (virtio_used_t*)(queue_zone.ptr + page_one_size);
    result->paddr = (uintptr_t)q_paddr_zone;
    return 0;
}