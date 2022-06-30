/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/devtree.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <libkern/types.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <tasking/tasking.h>

// #define DEBUG_SIMPLEFB

static kmemzone_t mapped_zone;
static char* simplefb_orig_buf_paddr = 0x0;
static char* simplefb_stub_buf_paddr[2];
static char* simplefb_stub_buf_vaddr[2];
static size_t simplefb_screen_width = 0x0;
static size_t simplefb_screen_height = 0x0;
static size_t simplefb_screen_buffer_size;
static size_t simplefb_one_buffer_size;

static inline uintptr_t _simplefb_mmio_paddr(devtree_entry_t* device)
{
    if (!device) {
        kpanic("simplefb: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_base;
}

static int _simplefb_swap_page_mode(struct memzone* zone, uintptr_t vaddr)
{
    return SWAP_NOT_ALLOWED;
}

static vm_ops_t mmap_file_vm_ops = {
    .load_page_content = NULL,
    .restore_swapped_page = NULL,
    .swap_page_mode = _simplefb_swap_page_mode,
};

static int _simplefb_init_buffer(device_t* dev)
{
    simplefb_screen_width = dev->device_desc.devtree.entry->aux1;
    simplefb_screen_height = dev->device_desc.devtree.entry->aux2;

    simplefb_one_buffer_size = simplefb_screen_width * 4 * simplefb_screen_height;
    simplefb_screen_buffer_size = simplefb_one_buffer_size * 2;
    char* paddr_zone = pmm_alloc(simplefb_screen_buffer_size);
    simplefb_stub_buf_paddr[0] = (char*)(paddr_zone);
    simplefb_stub_buf_paddr[1] = (char*)(paddr_zone + simplefb_one_buffer_size);

    mapped_zone = kmemzone_new(simplefb_screen_buffer_size);
    for (uintptr_t offset = 0; offset < simplefb_screen_buffer_size; offset += VMM_PAGE_SIZE) {
        vmm_map_page(mapped_zone.start + offset, (uintptr_t)(paddr_zone + offset), MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ);
    }

    simplefb_stub_buf_vaddr[0] = (char*)(mapped_zone.start);
    simplefb_stub_buf_vaddr[1] = (char*)(mapped_zone.start + simplefb_one_buffer_size);

    memset(mapped_zone.ptr, 0, simplefb_screen_buffer_size);
    return 0;
}

static void* memcpy64(uint64_t* dest, const uint64_t* src, size_t nbytes)
{
    for (int i = 0; i < nbytes / 8; i++) {
        dest[i] = src[i];
    }
    return dest;
}

static int swap_buffers(int id)
{
    // TODO(arm64): This is a hack, but we should use share vaddr between driver and screen.h
    extern volatile uint32_t* _fb;
    memcpy64((uint64_t*)_fb, (uint64_t*)simplefb_stub_buf_vaddr[id], simplefb_one_buffer_size);
    return 0;
}

static int _simplefb_ioctl(file_t* file, uintptr_t cmd, uintptr_t arg)
{
    switch (cmd) {
    case BGA_GET_HEIGHT:
        return simplefb_screen_height;
    case BGA_GET_WIDTH:
        return simplefb_screen_width;
    case BGA_GET_SCALE:
        // TODO: Determine based on screen size.
        return 1;
    case BGA_SWAP_BUFFERS:
        return swap_buffers(arg & 1);
    default:
        return -EINVAL;
    }
}

static memzone_t* _simplefb_mmap(file_t* file, mmap_params_t* params)
{
    bool map_shared = ((params->flags & MAP_SHARED) > 0);

    if (!map_shared) {
        return 0;
    }

    memzone_t* zone = memzone_new_random(RUNNING_THREAD->process->address_space, simplefb_screen_buffer_size);
    if (!zone) {
        return 0;
    }

    zone->mmu_flags |= MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ;
    zone->type |= ZONE_TYPE_DEVICE;
    zone->file = file_duplicate(file);
    zone->ops = &mmap_file_vm_ops;

    for (int offset = 0; offset < simplefb_screen_buffer_size; offset += VMM_PAGE_SIZE) {
        vmm_map_page(zone->vaddr + offset, (uintptr_t)(simplefb_stub_buf_paddr[0] + offset), zone->mmu_flags);
    }

    return zone;
}

static void simplefb_recieve_notification(uintptr_t msg, uintptr_t param)
{
    // Checking if device is inited
    if (!simplefb_orig_buf_paddr) {
        return;
    }

#ifdef DEBUG_SIMPLEFB
    log("simplefb: Notific start");
#endif
    if (msg == DEVMAN_NOTIFICATION_DEVFS_READY) {
        path_t vfspth;
        if (vfs_resolve_path("/dev", &vfspth) < 0) {
            kpanic("Can't init bga in /dev");
        }

        file_ops_t fops = { 0 };
        fops.ioctl = _simplefb_ioctl;
        fops.mmap = _simplefb_mmap;
        devfs_inode_t* res = devfs_register(&vfspth, MKDEV(10, 156), "bga", 3, S_IFBLK | 0777, &fops);

        path_put(&vfspth);
    }
#ifdef DEBUG_SIMPLEFB
    log("simplefb: Notific end");
#endif
}

int simplefb_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    simplefb_orig_buf_paddr = (char*)_simplefb_mmio_paddr(dev->device_desc.devtree.entry);
    _simplefb_init_buffer(dev);

#ifdef DEBUG_SIMPLEFB
    log("simplefb: Turning on: %p", simplefb_orig_buf_paddr);
#endif

    return 0;
}

static driver_desc_t _simplefb_driver_info()
{
    driver_desc_t simplefb_desc = { 0 };
    simplefb_desc.type = DRIVER_VIDEO_DEVICE;
    simplefb_desc.system_funcs.init_with_dev = simplefb_init;
    simplefb_desc.system_funcs.recieve_notification = simplefb_recieve_notification;
    simplefb_desc.functions[DRIVER_VIDEO_INIT] = simplefb_init;
    simplefb_desc.functions[DRIVER_VIDEO_SET_RESOLUTION] = NULL;
    return simplefb_desc;
}

void simplefb_install()
{
    devman_register_driver(_simplefb_driver_info(), "simplefb");
}
devman_register_driver_installation(simplefb_install);