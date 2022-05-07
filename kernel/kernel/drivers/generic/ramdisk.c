/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/devtree.h>
#include <drivers/generic/ramdisk.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>

// #define DEBUG_RAMDISK

#define RAMDISK_SECTOR_SIZE 512

static kmemzone_t mapped_zone;
static char* disk_base;

static inline uintptr_t _ramdisk_mmio_paddr(devtree_entry_t* device)
{
    if (!device) {
        kpanic("RamDisk: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_base;
}

static inline uintptr_t _ramdisk_mmio_size(devtree_entry_t* device)
{
    if (!device) {
        kpanic("RamDisk: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_size;
}

static inline int _ramdisk_map_itself(device_t* dev)
{
    log("RamDisk: %zx %zx", _ramdisk_mmio_paddr(dev->device_desc.devtree.entry), _ramdisk_mmio_size(dev->device_desc.devtree.entry));

    uintptr_t mmio_paddr = _ramdisk_mmio_paddr(dev->device_desc.devtree.entry);

    mapped_zone = kmemzone_new(_ramdisk_mmio_size(dev->device_desc.devtree.entry));
    size_t page_count = _ramdisk_mmio_size(dev->device_desc.devtree.entry) / VMM_PAGE_SIZE;
    vmm_map_pages(mapped_zone.start, mmio_paddr, page_count, MMU_FLAG_DEVICE);
    disk_base = (char*)mapped_zone.ptr;
    log("Mapped ramdisk");
    return 0;
}

int ramdisk_init_with_dev(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    if (_ramdisk_map_itself(dev)) {
#ifdef DEBUG_RAMDISK
        log_error("RamDisk: Can't map itself!");
#endif
        return -1;
    }

    log("RamDisk: added device");
    return 0;
}

void ramdisk_read(device_t* device, uint32_t lba_like, void* read_data)
{
    size_t offset = lba_like * RAMDISK_SECTOR_SIZE;
    memcpy(read_data, disk_base + offset, RAMDISK_SECTOR_SIZE);
}

void ramdisk_write(device_t* device, uint32_t lba_like, void* write_data)
{
    size_t offset = lba_like * RAMDISK_SECTOR_SIZE;
    memcpy(disk_base + offset, write_data, RAMDISK_SECTOR_SIZE);
}

static uint32_t ramdisk_capacity(device_t* device)
{
    return (uint32_t)mapped_zone.len;
}

static driver_desc_t _ramdisk_driver_info()
{
    driver_desc_t rd_desc = { 0 };
    rd_desc.type = DRIVER_STORAGE_DEVICE;
    rd_desc.listened_device_mask = DEVICE_STORAGE;

    rd_desc.system_funcs.init_with_dev = ramdisk_init_with_dev;

    rd_desc.functions[DRIVER_STORAGE_ADD_DEVICE] = ramdisk_init_with_dev;
    rd_desc.functions[DRIVER_STORAGE_READ] = ramdisk_read;
    rd_desc.functions[DRIVER_STORAGE_WRITE] = ramdisk_write;
    rd_desc.functions[DRIVER_STORAGE_FLUSH] = NULL;
    rd_desc.functions[DRIVER_STORAGE_CAPACITY] = ramdisk_capacity;
    return rd_desc;
}

void ramdisk_install()
{
    devman_register_driver(_ramdisk_driver_info(), "ramdisk");
}
devman_register_driver_installation(ramdisk_install);