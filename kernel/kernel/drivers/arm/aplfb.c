/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/pl111.h>
#include <drivers/devtree.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <tasking/tasking.h>

#define DEBUG_APLFB

static kmemzone_t mapped_zone;
static char* aplfb_orig_buf_paddr = 0x0;
static char* aplfb_stub_buf_paddr[2];
static char* aplfb_stub_buf_vaddr[2];
static size_t aplfb_screen_width = 1536;
static size_t aplfb_screen_height = 2048;
static size_t aplfb_screen_buffer_size;
static size_t aplfb_one_buffer_size;

static inline uintptr_t _aplfb_mmio_paddr(devtree_entry_t* device)
{
    if (!device) {
        kpanic("APLFB: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_base;
}

static int _aplfb_swap_page_mode(struct memzone* zone, uintptr_t vaddr)
{
    return SWAP_NOT_ALLOWED;
}

static vm_ops_t mmap_file_vm_ops = {
    .load_page_content = NULL,
    .restore_swapped_page = NULL,
    .swap_page_mode = _aplfb_swap_page_mode,
};

static int _aplfb_init_buffer()
{
    aplfb_one_buffer_size = aplfb_screen_width * 4 * aplfb_screen_height;
    aplfb_screen_buffer_size = aplfb_one_buffer_size * 2;
    char* paddr_zone = pmm_alloc(aplfb_screen_buffer_size);
    aplfb_stub_buf_paddr[0] = (char*)(paddr_zone);
    aplfb_stub_buf_paddr[1] = (char*)(paddr_zone + aplfb_one_buffer_size);

    mapped_zone = kmemzone_new(aplfb_screen_buffer_size);
    for (uintptr_t offset = 0; offset < aplfb_screen_buffer_size; offset += VMM_PAGE_SIZE) {
        vmm_map_page(mapped_zone.start + offset, (uintptr_t)(paddr_zone + offset), MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ);
    }

    aplfb_stub_buf_vaddr[0] = (char*)(mapped_zone.start);
    aplfb_stub_buf_vaddr[1] = (char*)(mapped_zone.start + aplfb_one_buffer_size);
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
    // TODO(aarch64): This is a hack, but we should use share vaddr between driver and screen.h
    extern volatile uint32_t* _fb;
    memcpy64((uint64_t*)_fb, (uint64_t*)aplfb_stub_buf_vaddr[id], aplfb_one_buffer_size);
    return 0;
}

static int _aplfb_ioctl(file_t* file, uintptr_t cmd, uintptr_t arg)
{
    switch (cmd) {
    case BGA_GET_HEIGHT:
        return aplfb_screen_height;
    case BGA_GET_WIDTH:
        return aplfb_screen_width;
    case BGA_SWAP_BUFFERS:
        return swap_buffers(arg & 1);
    default:
        return -EINVAL;
    }
}

static memzone_t* _aplfb_mmap(file_t* file, mmap_params_t* params)
{
    bool map_shared = ((params->flags & MAP_SHARED) > 0);

    if (!map_shared) {
        return 0;
    }

    memzone_t* zone = memzone_new_random(RUNNING_THREAD->process->address_space, aplfb_screen_buffer_size);
    if (!zone) {
        return 0;
    }

    zone->mmu_flags |= MMU_FLAG_PERM_WRITE | MMU_FLAG_PERM_READ;
    zone->type |= ZONE_TYPE_DEVICE;
    zone->file = file_duplicate(file);
    zone->ops = &mmap_file_vm_ops;

    for (int offset = 0; offset < aplfb_screen_buffer_size; offset += VMM_PAGE_SIZE) {
        vmm_map_page(zone->vaddr + offset, (uintptr_t)(aplfb_stub_buf_paddr[0] + offset), zone->mmu_flags);
    }

    return zone;
}

static void aplfb_recieve_notification(uintptr_t msg, uintptr_t param)
{
    // Checking if device is inited
    if (!aplfb_orig_buf_paddr) {
        return;
    }

#ifdef DEBUG_APLFB
    log("APLFB: Notific start");
#endif
    if (msg == DEVMAN_NOTIFICATION_DEVFS_READY) {
        path_t vfspth;
        if (vfs_resolve_path("/dev", &vfspth) < 0) {
            kpanic("Can't init bga in /dev");
        }

        file_ops_t fops = { 0 };
        fops.ioctl = _aplfb_ioctl;
        fops.mmap = _aplfb_mmap;
        devfs_inode_t* res = devfs_register(&vfspth, MKDEV(10, 156), "bga", 3, S_IFBLK | 0777, &fops);

        path_put(&vfspth);
    }
#ifdef DEBUG_APLFB
    log("APLFB: Notific end");
#endif
}

int aplfb_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    aplfb_orig_buf_paddr = (char*)_aplfb_mmio_paddr(dev->device_desc.devtree.entry);
    _aplfb_init_buffer();

#ifdef DEBUG_APLFB
    log("AplFB: Turning on: %p", aplfb_orig_buf_paddr);
#endif

    return 0;
}

static driver_desc_t _aplfb_driver_info()
{
    driver_desc_t aplfb_desc = { 0 };
    aplfb_desc.type = DRIVER_VIDEO_DEVICE;
    aplfb_desc.system_funcs.init_with_dev = aplfb_init;
    aplfb_desc.system_funcs.recieve_notification = aplfb_recieve_notification;
    aplfb_desc.functions[DRIVER_VIDEO_INIT] = aplfb_init;
    aplfb_desc.functions[DRIVER_VIDEO_SET_RESOLUTION] = NULL;
    return aplfb_desc;
}

void aplfb_install()
{
    devman_register_driver(_aplfb_driver_info(), "aplfb");
}
devman_register_driver_installation(aplfb_install);