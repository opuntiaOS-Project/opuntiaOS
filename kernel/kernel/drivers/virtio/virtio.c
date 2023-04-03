

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

#define MMIO_VIRTIO_STRIDE (0x1000)
#define MMIO_VIRTIO_MAGIC (0x74726976)

static kmemzone_t mapped_zone;
void* mapped_virtio = NULL;
size_t mapped_size;

static inline uintptr_t _virtio_mmio_paddr(devtree_entry_t* device)
{
    if (!device) {
        kpanic("VIRTIO: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_base;
}

static inline uintptr_t _virtio_mmio_size(devtree_entry_t* device)
{
    if (!device) {
        kpanic("VIRTIO: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_size;
}

static inline int _virtio_map_itself(device_t* dev)
{
    uintptr_t mmio_paddr = _virtio_mmio_paddr(dev->device_desc.devtree.entry);
    uintptr_t mmio_pages = (_virtio_mmio_size(dev->device_desc.devtree.entry) + VMM_PAGE_SIZE - 1) / VMM_PAGE_SIZE;

    mapped_zone = kmemzone_new(VMM_PAGE_SIZE * mmio_pages);
    vmm_map_pages(mapped_zone.start, mmio_paddr, mmio_pages, MMU_FLAG_DEVICE);
    mapped_virtio = mapped_zone.ptr;
    mapped_size = VMM_PAGE_SIZE * mmio_pages;
    return 0;
}

static void _virtio_add_gpu_device(void* ptr, size_t id, size_t width, size_t height)
{
    devtree_entry_t entry = {
        .region_base = (uintptr_t)ptr,
        .region_size = 0x1000,
        .type = DEVTREE_ENTRY_TYPE_FB,
        .rel_name_offset = devtree_new_entry_name("virtiogpu"),
        .flags = 0x0,
        .irq_lane = 1 + id,
        .irq_priority = 0x0,
    };

    device_desc_t new_device = { 0 };
    new_device.type = DEVICE_DESC_DEVTREE;
    new_device.devtree.entry = devtree_new_entry(&entry);
    new_device.args[0] = width;
    new_device.args[1] = height;
    devman_register_device(new_device, DEVICE_DISPLAY);
}

static void _virtio_add_block_device(void* ptr, size_t id)
{
    devtree_entry_t entry = {
        .region_base = (uintptr_t)ptr,
        .region_size = 0x1000,
        .type = DEVTREE_ENTRY_TYPE_STORAGE,
        .rel_name_offset = devtree_new_entry_name("virtioblock"),
        .flags = 0x0,
        .irq_lane = 1 + id,
        .irq_priority = 0x0,
    };

    device_desc_t new_device = { 0 };
    new_device.type = DEVICE_DESC_DEVTREE;
    new_device.devtree.entry = devtree_new_entry(&entry);
    devman_register_device(new_device, DEVICE_STORAGE);
}

static void _virtio_add_io_device(void* ptr, size_t id)
{
    devtree_entry_t entry = {
        .region_base = (uintptr_t)ptr,
        .region_size = 0x1000,
        .type = DEVTREE_ENTRY_TYPE_STORAGE,
        .rel_name_offset = devtree_new_entry_name("virtioinp"),
        .flags = 0x0,
        .irq_lane = 1 + id,
        .irq_priority = 0x0,
    };

    device_desc_t new_device = { 0 };
    new_device.type = DEVICE_DESC_DEVTREE;
    new_device.devtree.entry = devtree_new_entry(&entry);
    devman_register_device(new_device, DEVICE_INPUT_SYSTEMS);
}

static void _virtio_scan()
{
    for (size_t offset = 0; offset < mapped_size; offset += MMIO_VIRTIO_STRIDE) {
        volatile uint32_t* ptr = (uint32_t*)(mapped_virtio + offset);
        uint32_t magic = ptr[0];
        uint32_t deviceid = ptr[2];

        if (magic != MMIO_VIRTIO_MAGIC) {
            continue;
        }

        if (deviceid == 0) {
            continue;
        }

        switch (deviceid) {
        case VIRTIO_DEVTYPE_BLOCK:
            _virtio_add_block_device((void*)ptr, offset / MMIO_VIRTIO_STRIDE);
            break;

        case VIRTIO_DEVTYPE_GPU:
            _virtio_add_gpu_device((void*)ptr, offset / MMIO_VIRTIO_STRIDE, 1024, 768);
            break;

        case VIRTIO_DEVTYPE_IO:
            _virtio_add_io_device((void*)ptr, offset / MMIO_VIRTIO_STRIDE);
            break;

        default:
            break;
        }
    }
}

int virtio_init(device_t* dev)
{
    if (_virtio_map_itself(dev)) {
#ifdef DEBUG_VIRTIO
        log_error("VIRTIO: Can't map itself!");
#endif
        return -1;
    }

    virtio_alloc_init();
    _virtio_scan();
    return 0;
}

static driver_desc_t _virtio_driver_info()
{
    driver_desc_t vi_desc = { 0 };
    vi_desc.type = DRIVER_BUS_CONTROLLER;
    vi_desc.system_funcs.init_with_dev = virtio_init;
    vi_desc.system_funcs.recieve_notification = NULL;
    return vi_desc;
}

void virtio_install()
{
    devman_register_driver(_virtio_driver_info(), "virtio");
}

devman_register_driver_installation(virtio_install);