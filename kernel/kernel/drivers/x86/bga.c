/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/driver_manager.h>
#include <drivers/x86/bga.h>
#include <drivers/x86/pci.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <tasking/proc.h>
#include <tasking/tasking.h>

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF

#define VBE_DISPI_INDEX_ID 0x0
#define VBE_DISPI_INDEX_XRES 0x1
#define VBE_DISPI_INDEX_YRES 0x2
#define VBE_DISPI_INDEX_BPP 0x3
#define VBE_DISPI_INDEX_ENABLE 0x4
#define VBE_DISPI_INDEX_BANK 0x5
#define VBE_DISPI_INDEX_VIRT_WIDTH 0x6
#define VBE_DISPI_INDEX_VIRT_HEIGHT 0x7
#define VBE_DISPI_INDEX_X_OFFSET 0x8
#define VBE_DISPI_INDEX_Y_OFFSET 0x9

#define VBE_DISPI_DISABLED 0x00
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_LFB_ENABLED 0x40

static uint16_t bga_screen_width, bga_screen_height;
static uint32_t bga_screen_line_size, bga_screen_buffer_size;
static uint32_t bga_buf_paddr;

static inline void _bga_write_reg(uint16_t cmd, uint16_t data)
{
    port_16bit_out(VBE_DISPI_IOPORT_INDEX, cmd);
    port_16bit_out(VBE_DISPI_IOPORT_DATA, data);
}

static inline uint16_t _bga_read_reg(uint16_t cmd)
{
    port_16bit_out(VBE_DISPI_IOPORT_INDEX, cmd);
    return port_16bit_in(VBE_DISPI_IOPORT_DATA);
}

static void _bga_set_resolution(uint16_t width, uint16_t height)
{
    _bga_write_reg(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_DISABLED);
    _bga_write_reg(VBE_DISPI_INDEX_XRES, width);
    _bga_write_reg(VBE_DISPI_INDEX_YRES, height);
    _bga_write_reg(VBE_DISPI_INDEX_VIRT_WIDTH, width);
    _bga_write_reg(VBE_DISPI_INDEX_VIRT_HEIGHT, (uint16_t)height * 2);
    _bga_write_reg(VBE_DISPI_INDEX_BPP, 32);
    _bga_write_reg(VBE_DISPI_INDEX_X_OFFSET, 0);
    _bga_write_reg(VBE_DISPI_INDEX_Y_OFFSET, 0);
    _bga_write_reg(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);
    _bga_write_reg(VBE_DISPI_INDEX_BANK, 0);

    bga_screen_line_size = (uint32_t)width * 4;
}

static int _bga_ioctl(dentry_t* dentry, uint32_t cmd, uint32_t arg)
{
    uint32_t y_offset = 0;
    switch (cmd) {
    case BGA_GET_HEIGHT:
        return bga_screen_height;
    case BGA_GET_WIDTH:
        return bga_screen_width;
    case BGA_SWAP_BUFFERS:
        y_offset = bga_screen_height * (arg & 1);
        _bga_write_reg(VBE_DISPI_INDEX_Y_OFFSET, (uint16_t)y_offset);
        return 0;
    default:
        return -EINVAL;
    }
}

static memzone_t* _bga_mmap(dentry_t* dentry, mmap_params_t* params)
{
    bool map_shared = ((params->flags & MAP_SHARED) > 0);

    if (!map_shared) {
        return 0;
    }

    memzone_t* zone = memzone_new_random(RUNNING_THREAD->process, bga_screen_buffer_size);
    if (!zone) {
        return 0;
    }

    zone->flags |= ZONE_WRITABLE | ZONE_READABLE | ZONE_NOT_CACHEABLE;
    zone->type |= ZONE_TYPE_DEVICE;
    zone->file = dentry_duplicate(dentry);

    for (int offset = 0; offset < bga_screen_buffer_size; offset += VMM_PAGE_SIZE) {
        vmm_map_page(zone->start + offset, bga_buf_paddr + offset, zone->flags);
    }

    return zone;
}

static void bga_recieve_notification(uint32_t msg, uint32_t param)
{
    if (msg == DM_NOTIFICATION_DEVFS_READY) {
        dentry_t* mp;
        if (vfs_resolve_path("/dev", &mp) < 0) {
            kpanic("Can't init bga in /dev");
        }

        file_ops_t fops = { 0 };
        fops.ioctl = _bga_ioctl;
        fops.mmap = _bga_mmap;
        devfs_inode_t* res = devfs_register(mp, MKDEV(10, 156), "bga", 3, 0777, &fops);

        dentry_put(mp);
    }
}

static inline driver_desc_t _bga_driver_info()
{
    driver_desc_t bga_desc = { 0 };
    bga_desc.type = DRIVER_VIDEO_DEVICE;
    bga_desc.auto_start = false;
    bga_desc.is_device_driver = true;
    bga_desc.is_device_needed = false;
    bga_desc.is_driver_needed = false;
    bga_desc.functions[DRIVER_NOTIFICATION] = bga_recieve_notification;
    bga_desc.functions[DRIVER_VIDEO_INIT] = bga_init;
    bga_desc.functions[DRIVER_VIDEO_SET_RESOLUTION] = bga_set_resolution;
    bga_desc.pci_serve_class = 0x03;
    bga_desc.pci_serve_subclass = 0x00;
    bga_desc.pci_serve_vendor_id = 0x1234;
    bga_desc.pci_serve_device_id = 0x1111;
    return bga_desc;
}

void bga_install()
{
    driver_install(_bga_driver_info(), "bga86");
}

void bga_init(device_t* dev)
{
    bga_buf_paddr = pci_read_bar(dev, 0) & 0xfffffff0;
#ifdef TARGET_DESKTOP
    bga_set_resolution(1024, 768);
#elif TARGET_MOBILE
    bga_set_resolution(320, 568);
#endif
}

void bga_set_resolution(uint16_t width, uint16_t height)
{
    _bga_set_resolution(width, height);
    bga_screen_width = width;
    bga_screen_height = height;
    bga_screen_buffer_size = bga_screen_line_size * (uint32_t)height * 2;
}