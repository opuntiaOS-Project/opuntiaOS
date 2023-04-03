/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/devtree.h>
#include <drivers/irq/irq_api.h>
#include <drivers/storage/virtio_block.h>
#include <drivers/virtio/virtio.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <libkern/types.h>
#include <mem/kmalloc.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <tasking/tasking.h>

// #define DEBUG_VIRTIO_BLOCK

static int block_dev_count = 0;
block_dev_t block_dev;

#define ADD_TO_QUEUE(dev, wt)                     \
    dev.queue_desc.descs->entities[dev.idx] = wt; \
    dev.idx = (dev.idx + 1) % VIRTIO_RING_SIZE

#define SET_AS_AVAIL(dev, hd)                                                      \
    dev.queue_desc.avail->ring[dev.queue_desc.avail->idx % VIRTIO_RING_SIZE] = hd; \
    dev.queue_desc.avail->idx++

static int virtio_block_dev_rw(char* buffer, size_t lba, size_t len, bool write)
{
    // TODO: We need to use paddr for request. There is a function in vmm (vmm_convert_kernel_vaddr_to_paddr) which
    // works perfectly for kernel addresses (where this given buffer should be), but we might get a deadlock,
    // since VMM calls this driver for page fill up, while holding the lock (see vmm_resolve_page_not_present_locked).
    // This could be fixed with reentrant locks for VMM.
    if (write) {
        memcpy(block_dev.buffer_desc.ptr, buffer, 512);
    }

    size_t sector = lba;
    block_request_t req1 = {
        .header.sector = sector,
        .header.blktype = (write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN),
        .header.reserved = 0,
        .status.status = 111,
        .watcher = 0,
    };

    virtio_alloc_result_t alloc_result;
    int err = virtio_alloc_raw(&req1, &alloc_result);
    if (err) {
#ifdef DEBUG_VIRTIO_BLOCK
        log_warn("VIRTIO_BLOCK: Error in virtio_alloc_raw");
#endif
        return 1;
    }
    block_request_t* req_paddr = (block_request_t*)alloc_result.req_paddr;
    block_request_t* req_vaddr = (block_request_t*)alloc_result.req_vaddr;

    virtio_desc_t desc = {
        .addr = (uintptr_t)(&(req_paddr->header)),
        .len = sizeof(block_header_t),
        .flags = VIRTIO_DESC_F_NEXT,
        .next = (block_dev.idx + 1) % VIRTIO_RING_SIZE,
    };
    virtio_desc_t desc2 = {
        .addr = block_dev.buffer_desc.paddr,
        .len = len,
        .flags = !write ? VIRTIO_DESC_F_NEXT | VIRTIO_DESC_F_WRITE : VIRTIO_DESC_F_NEXT,
        .next = (block_dev.idx + 2) % VIRTIO_RING_SIZE,
    };
    virtio_desc_t desc3 = {
        .addr = (uintptr_t)(&(req_paddr->status.status)),
        .len = sizeof(block_status_t),
        .flags = VIRTIO_DESC_F_WRITE,
        .next = 0,
    };

    uint32_t head = block_dev.idx;
    ADD_TO_QUEUE(block_dev, desc);
    ADD_TO_QUEUE(block_dev, desc2);
    ADD_TO_QUEUE(block_dev, desc3);
    SET_AS_AVAIL(block_dev, head);

    volatile virtio_mmio_registers_t* registers = (virtio_mmio_registers_t*)block_dev.ptr;
    registers->queue_notify = 0;

    volatile uint8_t* status = &(req_vaddr->status.status);
    while (*status == 111) { }

    if (!write) {
        memcpy(buffer, block_dev.buffer_desc.ptr, 512);
    }
    return 0;
}

static void _virtioblock_int_handler()
{
    while (block_dev.ack_used_idx != block_dev.queue_desc.used->idx) {
        int id = block_dev.queue_desc.used->ring[block_dev.ack_used_idx % VIRTIO_RING_SIZE].id;
        virtio_free_paddr((void*)(uintptr_t)block_dev.queue_desc.descs->entities[id].addr);
        block_dev.ack_used_idx++;
    }
}

static int virtioblock_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    block_dev_count++;

    irq_register_handler(dev->device_desc.devtree.entry->irq_lane, dev->device_desc.devtree.entry->irq_priority,
        (irq_flags_t)0x0, _virtioblock_int_handler, BOOT_CPU_MASK);

    uintptr_t mmio_vaddr = dev->device_desc.devtree.entry->region_base;
    volatile virtio_mmio_registers_t* registers = (virtio_mmio_registers_t*)mmio_vaddr;
    registers->status = 0;

    uint32_t status_bits = VIRTIO_STATUS_ACK | VIRTIO_STATUS_DRIVER_OK;
    registers->status = status_bits;

    uint32_t host_feats = registers->host_features & !(1 << VIRTIO_BLK_F_RO);
    registers->guest_features = host_feats;

    status_bits |= VIRTIO_STATUS_FEATS_OK;
    registers->status = status_bits;

    uint32_t read_status = registers->status;
    if (!TEST_FLAG(read_status, VIRTIO_STATUS_FEATS_OK)) {
#ifdef DEBUG_VIRTIO_BLOCK
        log_warn("VIRTIO_BLOCK: Can't init");
#endif
        return 1;
    }

    uint32_t qnmax = registers->queue_num_max;
    registers->queue_num = VIRTIO_RING_SIZE;
    if (VIRTIO_RING_SIZE > qnmax) {
#ifdef DEBUG_VIRTIO_BLOCK
        log_warn("VIRTIO_BLOCK: Ring cannot be inited");
#endif
        return 1;
    }

    virtio_queue_desc_t queue;
    int err = virtio_alloc_queue(&queue);
    if (err) {
        return err;
    }

    registers->queue_sel = 0;
    registers->guest_page_size = VMM_PAGE_SIZE;
    registers->queue_pfn = queue.paddr / VMM_PAGE_SIZE;

    status_bits |= VIRTIO_STATUS_DRIVER_OK;
    registers->status = status_bits;

    size_t rw_buffer_size = VMM_PAGE_SIZE;
    virtio_buffer_desc_t rw_buffer;
    err = virtio_alloc_buffer(rw_buffer_size, &rw_buffer);
    if (err) {
        return err;
    }

    block_dev.queue_desc = queue;
    block_dev.ptr = (void*)registers;
    block_dev.idx = 0;
    block_dev.ack_used_idx = 0;
    block_dev.buffer_desc = rw_buffer;
    return 0;
}

static int _virtioblock_read_block(device_t* device, uint32_t lba_like, void* read_data)
{
    return virtio_block_dev_rw(read_data, lba_like, 512, false);
}

static int _virtioblock_write_block(device_t* device, uint32_t lba_like, void* write_data)
{
    return virtio_block_dev_rw(write_data, lba_like, 512, true);
}

static uint32_t _virtioblock_get_capacity(device_t* device)
{
    // TODO: Get real size.
    return (128 << 20);
}

static driver_desc_t _virtioblock_driver_info()
{
    driver_desc_t virtioblock_desc = { 0 };
    virtioblock_desc.type = DRIVER_STORAGE_DEVICE;
    virtioblock_desc.listened_device_mask = DEVICE_STORAGE;

    virtioblock_desc.system_funcs.init_with_dev = virtioblock_init;

    virtioblock_desc.functions[DRIVER_STORAGE_ADD_DEVICE] = virtioblock_init;
    virtioblock_desc.functions[DRIVER_STORAGE_READ] = _virtioblock_read_block;
    virtioblock_desc.functions[DRIVER_STORAGE_WRITE] = _virtioblock_write_block;
    virtioblock_desc.functions[DRIVER_STORAGE_FLUSH] = NULL;
    virtioblock_desc.functions[DRIVER_STORAGE_CAPACITY] = _virtioblock_get_capacity;
    return virtioblock_desc;
}

void virtioblock_install()
{
    devman_register_driver(_virtioblock_driver_info(), "virtioblock");
}
devman_register_driver_installation(virtioblock_install);
