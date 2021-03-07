/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/pl111.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>
#include <tasking/tasking.h>

#define DEBUG_PL111

static zone_t mapped_zone;
static volatile pl111_registers_t* registers = (pl111_registers_t*)PL111_BASE;
static char* pl111_bufs_paddr[2];
static uint32_t pl111_screen_width;
static uint32_t pl111_screen_height;
static uint32_t pl111_screen_buffer_size;

static inline int _pl111_map_itself()
{
    mapped_zone = zoner_new_zone(sizeof(pl111_registers_t));
    vmm_map_page(mapped_zone.start, PL111_BASE, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE);
    registers = (pl111_registers_t*)mapped_zone.ptr;
    return 0;
}

static inline uint32_t roundup_to_page_size(uint32_t size)
{
    if (size % VMM_PAGE_SIZE) {
        size += VMM_PAGE_SIZE - (size % VMM_PAGE_SIZE);
    }
    return size;
}

static int _pl111_init_buffer(uint32_t width, uint32_t height)
{
    uint32_t one_screen_len = width * 4 * height;
    pl111_screen_buffer_size = one_screen_len * 2;
    char* paddr_zone = pmm_alloc(pl111_screen_buffer_size);
    pl111_bufs_paddr[0] = (char*)(paddr_zone);
    pl111_bufs_paddr[1] = (char*)(paddr_zone + one_screen_len);
    registers->lcd_upbase = (uint32_t)pl111_bufs_paddr[0];
}

static int _pl111_ioctl(dentry_t* dentry, uint32_t cmd, uint32_t arg)
{
    uint32_t y_offset = 0;
    switch (cmd) {
    case 0x1:
        log("%x", arg);
        return 0;
    case BGA_SWAP_BUFFERS:
        registers->lcd_upbase = (uint32_t)pl111_bufs_paddr[(arg & 1)];
        return 0;
    default:
        return -EINVAL;
    }
}

static proc_zone_t* _pl111_mmap(dentry_t* dentry, mmap_params_t* params)
{
    bool map_shared = ((params->flags & MAP_SHARED) > 0);

    if (!map_shared) {
        return 0;
    }

    proc_zone_t* zone = proc_new_random_zone(RUNNIG_THREAD->process, pl111_screen_buffer_size);
    if (!zone) {
        return 0;
    }

    zone->flags |= ZONE_WRITABLE | ZONE_READABLE;

    for (int offset = 0; offset < pl111_screen_buffer_size; offset += VMM_PAGE_SIZE) {
        vmm_map_page(zone->start + offset, (uint32_t)(pl111_bufs_paddr[0] + offset), zone->flags);
    }

    return zone;
}

static void pl111_recieve_notification(uint32_t msg, uint32_t param)
{
    log("PL111: Notific start");
    if (msg == DM_NOTIFICATION_DEVFS_READY) {
        dentry_t* mp;
        if (vfs_resolve_path("/dev", &mp) < 0) {
            kpanic("Can't init bga in /dev");
        }

        file_ops_t fops = { 0 };
        fops.ioctl = _pl111_ioctl;
        fops.mmap = _pl111_mmap;
        devfs_inode_t* res = devfs_register(mp, MKDEV(10, 156), "bga", 3, 0, &fops);

        dentry_put(mp);
    }
    log("PL111: Notific end");
}

void pl111_set_resolution(uint32_t width, uint32_t height)
{
    size_t ppl = width / 16 - 1;
    volatile uint32_t timing_reg = registers->lcd_timing_0;
    timing_reg &= ~PIXELS_PER_LINE_MASK;
    timing_reg |= (ppl << PIXELS_PER_LINE_POS) & PIXELS_PER_LINE_MASK;
    registers->lcd_timing_0 = timing_reg;

    timing_reg = registers->lcd_timing_1;
    timing_reg &= ~LINES_PER_PANEL_MASK;
    timing_reg |= ((height - 1) << LINES_PER_PANEL_POS) & LINES_PER_PANEL_MASK;
    registers->lcd_timing_1 = timing_reg;
}

void pl111_init(device_t* dev)
{
#ifdef DEBUG_PL111
    log("PL111: Turning on");
#endif
    _pl111_init_buffer(1024, 768);
    pl111_set_resolution(1024, 768);

    volatile uint32_t ctl = registers->lcd_control;
    ctl &= ~LCD_POWER_MASK
        | ~LCD_BGR_MASK
        | ~LCD_BPP_MASK
        | ~LCD_EN_MASK;

    ctl |= LCD_POWER_MASK
        | LCD_BGR_MASK
        | LCD_TFT_MASK
        | (LCD_24_BPP << LCD_BPP_POS)
        | LCD_EN_MASK;

    registers->lcd_control = ctl;
}

static driver_desc_t _pl111_driver_info()
{
    driver_desc_t pl111_desc = { 0 };
    pl111_desc.type = DRIVER_VIDEO_DEVICE;
    pl111_desc.auto_start = true;
    pl111_desc.is_device_driver = false;
    pl111_desc.is_device_needed = false;
    pl111_desc.is_driver_needed = false;
    pl111_desc.functions[DRIVER_NOTIFICATION] = pl111_recieve_notification;
    pl111_desc.functions[DRIVER_VIDEO_INIT] = pl111_init;
    pl111_desc.functions[DRIVER_VIDEO_SET_RESOLUTION] = pl111_set_resolution;
    pl111_desc.pci_serve_class = 0x00;
    pl111_desc.pci_serve_subclass = 0x00;
    pl111_desc.pci_serve_vendor_id = 0x1234;
    pl111_desc.pci_serve_device_id = 0x1111;
    return pl111_desc;
}

void pl111_install()
{
    if (_pl111_map_itself()) {
#ifdef DEBUG_PL111
        log_error("PL111: Can't map itself!");
#endif
        return;
    }

    driver_install(_pl111_driver_info(), "pl111");
}