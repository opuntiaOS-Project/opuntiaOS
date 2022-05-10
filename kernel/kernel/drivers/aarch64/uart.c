/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/devtree.h>
#include <drivers/driver_manager.h>
#include <libkern/types.h>
#include <mem/boot.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>

static kmemzone_t mapped_zone;
volatile uint8_t* uart = NULL;

void uart_setup(boot_args_t* boot_args)
{
    devtree_entry_t* device = devtree_find_device("uart");
    if (device) {
        uart = (uint8_t*)device->region_base;
    }
}

int uart_write(uint8_t data)
{
    if (!uart) {
        return -1;
    }
    *uart = data;
    return 0;
}

static inline int _uart_map_itself()
{
    // Paddr is taken from uart which is set during setup.
    uintptr_t mmio_paddr = (uintptr_t)uart;

    mapped_zone = kmemzone_new(VMM_PAGE_SIZE);
    vmm_map_page(mapped_zone.start, mmio_paddr, MMU_FLAG_DEVICE);
    uart = (uint8_t*)mapped_zone.ptr;
    return 0;
}

void uart_remap()
{
    if (!uart) {
        return;
    }
    _uart_map_itself();
}
devman_register_driver_installation_order(uart_remap, 10);
