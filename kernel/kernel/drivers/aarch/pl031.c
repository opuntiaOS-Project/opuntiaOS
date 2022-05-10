/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/pl031.h>
#include <drivers/devtree.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <tasking/tasking.h>

// #define DEBUG_PL031

static kmemzone_t mapped_zone;
static volatile pl031_registers_t* registers;

static inline uintptr_t _pl031_mmio_paddr()
{
    devtree_entry_t* device = devtree_find_device("pl031");
    if (!device) {
        kpanic("PL031: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_base;
}

static inline int _pl031_map_itself()
{
    uintptr_t mmio_paddr = _pl031_mmio_paddr();

    mapped_zone = kmemzone_new(sizeof(pl031_registers_t));
    vmm_map_page(mapped_zone.start, mmio_paddr, MMU_FLAG_DEVICE);
    registers = (pl031_registers_t*)mapped_zone.ptr;
    return 0;
}

int pl031_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    if (_pl031_map_itself()) {
#ifdef DEBUG_PL031
        log_error("PL031: Can't map itself!");
#endif
        return -1;
    }

    return 0;
}

uint32_t pl031_read_rtc()
{
    return registers->data;
}

static driver_desc_t _pl031_driver_info()
{
    driver_desc_t pl031_desc = { 0 };
    pl031_desc.type = DRIVER_RTC;
    pl031_desc.system_funcs.init_with_dev = pl031_init;
    pl031_desc.functions[DRIVER_RTC_GET_TIME] = pl031_read_rtc;
    return pl031_desc;
}

void pl031_install()
{
    devman_register_driver(_pl031_driver_info(), "pl031");
}
devman_register_driver_installation(pl031_install);