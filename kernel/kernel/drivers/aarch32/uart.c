/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/uart.h>
#include <drivers/devtree.h>
#include <drivers/driver_manager.h>
#include <mem/boot.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>

volatile uint32_t* output = (uint32_t*)COM1;
static kmemzone_t mapped_zone;

static inline uintptr_t _uart_mmio_paddr()
{
    devtree_entry_t* device = devtree_find_device("uart");
    if (!device) {
        kpanic("UART: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_base;
}

static inline int _uart_map_itself()
{
    uintptr_t mmio_paddr = _uart_mmio_paddr();

    mapped_zone = kmemzone_new(VMM_PAGE_SIZE);
    vmm_map_page(mapped_zone.start, mmio_paddr, MMU_FLAG_DEVICE);
    output = (uint32_t*)mapped_zone.ptr;
    return 0;
}

/* No setup for arm version now */
void uart_setup(boot_args_t* boot_args)
{
}

void uart_remap()
{
    _uart_map_itself();
}
devman_register_driver_installation_order(uart_remap, 10);

int uart_write(uint8_t data)
{
    *output = data;
    return 0;
}

int uart_read(uint8_t* data)
{
    return 0;
}