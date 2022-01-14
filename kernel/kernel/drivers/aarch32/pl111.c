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

#define DEBUG_PL111

static kmemzone_t mapped_zone;
static volatile pl111_registers_t* registers;
static char* pl111_bufs_paddr[2];
static uint32_t pl111_screen_width;
static uint32_t pl111_screen_height;
static uint32_t pl111_screen_buffer_size;

static inline uintptr_t _pl111_mmio_paddr(devtree_entry_t* device)
{
    if (!device) {
        kpanic("PL111: Can't find device in the tree.");
    }

    return (uintptr_t)device->paddr;
}

static int _pl111_swap_page_mode(struct memzone* zone, uintptr_t vaddr)
{
    return SWAP_NOT_ALLOWED;
}

static vm_ops_t mmap_file_vm_ops = {
    .load_page_content = NULL,
    .restore_swapped_page = NULL,
    .swap_page_mode = _pl111_swap_page_mode,
};

static inline int _pl111_map_itself(device_t* dev)
{
    uintptr_t mmio_paddr = _pl111_mmio_paddr(dev->device_desc.devtree.entry);

    mapped_zone = kmemzone_new(sizeof(pl111_registers_t));
    vmm_map_page(mapped_zone.start, mmio_paddr, PAGE_DEVICE);
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
    return 0;
}

static int _pl111_ioctl(dentry_t* dentry, uint32_t cmd, uint32_t arg)
{
    switch (cmd) {
    case BGA_GET_HEIGHT:
        return pl111_screen_height;
    case BGA_GET_WIDTH:
        return pl111_screen_width;
    case BGA_SWAP_BUFFERS:
        registers->lcd_upbase = (uint32_t)pl111_bufs_paddr[(arg & 1)];
        return 0;
    default:
        return -EINVAL;
    }
}

static memzone_t* _pl111_mmap(dentry_t* dentry, mmap_params_t* params)
{
    bool map_shared = ((params->flags & MAP_SHARED) > 0);

    if (!map_shared) {
        return 0;
    }

    memzone_t* zone = memzone_new_random(RUNNING_THREAD->process, pl111_screen_buffer_size);
    if (!zone) {
        return 0;
    }

    zone->flags |= ZONE_WRITABLE | ZONE_READABLE | ZONE_NOT_CACHEABLE;
    zone->type |= ZONE_TYPE_DEVICE;
    zone->file = dentry_duplicate(dentry);
    zone->ops = &mmap_file_vm_ops;

    for (int offset = 0; offset < pl111_screen_buffer_size; offset += VMM_PAGE_SIZE) {
        vmm_map_page(zone->start + offset, (uint32_t)(pl111_bufs_paddr[0] + offset), zone->flags);
    }

    return zone;
}

static void pl111_recieve_notification(uint32_t msg, uint32_t param)
{
#ifdef DEBUG_PL111
    log("PL111: Notific start");
#endif
    if (msg == DEVMAN_NOTIFICATION_DEVFS_READY) {
        dentry_t* mp;
        if (vfs_resolve_path("/dev", &mp) < 0) {
            kpanic("Can't init bga in /dev");
        }

        file_ops_t fops = { 0 };
        fops.ioctl = _pl111_ioctl;
        fops.mmap = _pl111_mmap;
        devfs_inode_t* res = devfs_register(mp, MKDEV(10, 156), "bga", 3, 0777, &fops);

        dentry_put(mp);
    }
#ifdef DEBUG_PL111
    log("PL111: Notific end");
#endif
}

void pl111_set_resolution(uint32_t width, uint32_t height)
{
    pl111_screen_width = width;
    pl111_screen_height = height;

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

int pl111_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    if (_pl111_map_itself(dev)) {
#ifdef DEBUG_PL111
        log_error("PL111: Can't map itself!");
#endif
        return -1;
    }

#ifdef DEBUG_PL111
    log("PL111: Turning on");
#endif
#ifdef TARGET_DESKTOP
    _pl111_init_buffer(1024, 768);
    pl111_set_resolution(1024, 768);
#elif TARGET_MOBILE
    _pl111_init_buffer(320, 568);
    pl111_set_resolution(320, 568);
#endif

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
    return 0;
}

static driver_desc_t _pl111_driver_info()
{
    driver_desc_t pl111_desc = { 0 };
    pl111_desc.type = DRIVER_VIDEO_DEVICE;
    pl111_desc.system_funcs.init_with_dev = pl111_init;
    pl111_desc.system_funcs.recieve_notification = pl111_recieve_notification;
    pl111_desc.functions[DRIVER_VIDEO_INIT] = pl111_init;
    pl111_desc.functions[DRIVER_VIDEO_SET_RESOLUTION] = pl111_set_resolution;
    return pl111_desc;
}

void pl111_install()
{
    devman_register_driver(_pl111_driver_info(), "pl111");
}
devman_register_driver_installation(pl111_install);