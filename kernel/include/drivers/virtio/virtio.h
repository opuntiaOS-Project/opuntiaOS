/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_VIRTIO_VIRTIO_H
#define _KERNEL_DRIVERS_VIRTIO_VIRTIO_H

#include <drivers/driver_manager.h>
#include <libkern/mask.h>
#include <libkern/types.h>
#include <platform/generic/vmm/consts.h>
#include <time/time_manager.h>

#define VIRTIO_DESC_F_NEXT (1)
#define VIRTIO_DESC_F_WRITE (2)
#define VIRTIO_DESC_F_INDIRECT (4)

#define VIRTIO_F_EVENT_IDX (1 << 29)

#define VIRTIO_RING_SIZE (1 << 7)

enum VIRTIO_DEVTYPE {
    VIRTIO_DEVTYPE_NETWORK = 1,
    VIRTIO_DEVTYPE_BLOCK = 2,
    VIRTIO_DEVTYPE_GPU = 16,
    VIRTIO_DEVTYPE_IO = 18,
};

enum VIRTIO_STATUS {
    VIRTIO_STATUS_ACK = 1,
    VIRTIO_STATUS_DRIVER = 2,
    VIRTIO_STATUS_DRIVER_OK = 4,
    VIRTIO_STATUS_FEATS_OK = 8,
    VIRTIO_STATUS_DEV_NEEDS_RESET = 64,
    VIRTIO_STATUS_FAILED = 128,
};

struct virtio_mmio_registers {
    uint32_t magic_value;
    uint32_t version;
    uint32_t device_id;
    uint32_t vendor_id;
    uint32_t host_features;
    uint32_t host_features_sel;
    uint8_t reserved1[8];
    uint32_t guest_features;
    uint32_t guest_features_sel;
    uint32_t guest_page_size;
    uint8_t reserved2[4];
    uint32_t queue_sel;
    uint32_t queue_num_max;
    uint32_t queue_num;
    uint32_t queue_align;
    uint64_t queue_pfn;
    uint8_t reserved3[8];
    uint32_t queue_notify;
    uint8_t reserved4[12];
    uint32_t interrupt_status;
    uint32_t interrupt_ack;
    uint8_t reserved5[8];
    uint32_t status;
    uint8_t reserved6[140];
    uint32_t config[1];
};
typedef struct virtio_mmio_registers virtio_mmio_registers_t;

struct virtio_desc {
    uint64_t addr;
    uint32_t len;
    uint16_t flags;
    uint16_t next;
};
typedef struct virtio_desc virtio_desc_t;

struct virtio_avail {
    uint16_t flags;
    uint16_t idx;
    uint16_t ring[VIRTIO_RING_SIZE];
    uint16_t event;
};
typedef struct virtio_avail virtio_avail_t;

struct virtio_used_elem {
    uint32_t id;
    uint32_t len;
};
typedef struct virtio_used_elem virtio_used_elem_t;

struct virtio_used {
    uint16_t flags;
    uint16_t idx;
    virtio_used_elem_t ring[VIRTIO_RING_SIZE];
    uint16_t event;
};
typedef struct virtio_used virtio_used_t;

struct virtio_desc_array {
    virtio_desc_t entities[VIRTIO_RING_SIZE];
};
typedef struct virtio_desc_array virtio_desc_array_t;

struct virtio_queue {
    virtio_desc_t desc[VIRTIO_RING_SIZE];
    virtio_avail_t avail;
    uint8_t padding0[4096 - sizeof(virtio_desc_t) * VIRTIO_RING_SIZE - sizeof(virtio_avail_t)];
    virtio_used_t used;
};
typedef struct virtio_queue virtio_queue_t;

struct virtio_alloc_result {
    void* req_vaddr;
    void* req_paddr;
    void* mem_vaddr;
    void* mem_paddr;
    void* resp_vaddr;
    void* resp_paddr;
};
typedef struct virtio_alloc_result virtio_alloc_result_t;

struct virtio_buffer_desc {
    union {
        uintptr_t vaddr;
        void* ptr;
    };
    uintptr_t paddr;
    kmemzone_t kzone;
};
typedef struct virtio_buffer_desc virtio_buffer_desc_t;

struct virtio_queue_desc {
    // These are vaddrs.
    virtio_desc_array_t* descs;
    virtio_avail_t* avail;
    virtio_used_t* used;

    // Start of the queue.
    uintptr_t paddr;
};
typedef struct virtio_queue_desc virtio_queue_desc_t;

int virtio_alloc_buffer(size_t size, virtio_buffer_desc_t* result);
int virtio_alloc_queue(virtio_queue_desc_t* result);

int virtio_alloc_init();
int virtio_alloc(size_t size, virtio_alloc_result_t* result);
void virtio_free_paddr(void* paddr_ptr);

#define virtio_alloc_raw(req_ptr, result) do_virtio_alloc_raw(req_ptr, sizeof(*req_ptr), result)
static int do_virtio_alloc_raw(void* req_ptr, size_t req_size, virtio_alloc_result_t* result)
{
    int err = virtio_alloc(req_size, result);
    if (err) {
        return err;
    }
    memcpy(result->req_vaddr, req_ptr, req_size);
    return err;
}

#define virtio_alloc_request(req_ptr, resp_ptr, result) do_virtio_alloc_request(req_ptr, sizeof(*req_ptr), resp_ptr, sizeof(*resp_ptr), result)
static int do_virtio_alloc_request(void* req_ptr, size_t req_size, void* resp_ptr, size_t resp_size, virtio_alloc_result_t* result)
{
    int err = virtio_alloc(req_size + resp_size, result);
    if (err) {
        return err;
    }
    result->resp_vaddr = (void*)((uintptr_t)result->req_vaddr + req_size);
    result->resp_paddr = (void*)((uintptr_t)result->req_paddr + req_size);
    memcpy(result->req_vaddr, req_ptr, req_size);
    memcpy(result->resp_vaddr, resp_ptr, resp_size);
    return err;
}

#define virtio_alloc_mem_request(req_ptr, mem_ptr, resp_ptr, result) do_alloc_mem_request(req_ptr, sizeof(*req_ptr), mem_ptr, sizeof(*mem_ptr), resp_ptr, sizeof(*resp_ptr), result)
static int do_alloc_mem_request(void* req_ptr, size_t req_size, void* mem_ptr, size_t mem_size, void* resp_ptr, size_t resp_size, virtio_alloc_result_t* result)
{
    int err = virtio_alloc(req_size + mem_size + resp_size, result);
    if (err) {
        return err;
    }
    result->mem_vaddr = (void*)((uintptr_t)result->req_vaddr + req_size);
    result->mem_paddr = (void*)((uintptr_t)result->req_paddr + req_size);
    result->resp_vaddr = (void*)((uintptr_t)result->mem_vaddr + mem_size);
    result->resp_paddr = (void*)((uintptr_t)result->mem_paddr + mem_size);
    memcpy(result->req_vaddr, req_ptr, req_size);
    memcpy(result->mem_vaddr, mem_ptr, mem_size);
    memcpy(result->resp_vaddr, resp_ptr, resp_size);
    return err;
}

#endif //_KERNEL_DRIVERS_VIRTIO_VIRTIO_H