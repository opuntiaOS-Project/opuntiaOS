/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/pl050.h>
#include <drivers/devtree.h>
#include <drivers/generic/keyboard.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <platform/aarch32/interrupts.h>

// #define KEYBOARD_DRIVER_DEBUG
#ifdef KEYBOARD_DRIVER_DEBUG
#define DEBUG_PL050
#endif

static kmemzone_t mapped_zone;
static volatile pl050_registers_t* registers = 0x0;

static inline uintptr_t _pl050_mmio_paddr(devtree_entry_t* device)
{
    if (!device) {
        kpanic("PL050 KBD: Can't find device in the tree.");
    }

    return (uintptr_t)device->region_base;
}

static inline int _pl050_map_itself(device_t* dev)
{
    uintptr_t mmio_paddr = _pl050_mmio_paddr(dev->device_desc.devtree.entry);

    mapped_zone = kmemzone_new(sizeof(pl050_registers_t));
    vmm_map_page(mapped_zone.start, mmio_paddr, MMU_FLAG_DEVICE);
    registers = (pl050_registers_t*)mapped_zone.ptr;
    return 0;
}

static void pl050_keyboard_recieve_notification(uintptr_t msg, uintptr_t param)
{
    // Checking if device is inited
    if (!registers) {
        return;
    }

    if (msg == DEVMAN_NOTIFICATION_DEVFS_READY) {
        if (msg == DEVMAN_NOTIFICATION_DEVFS_READY) {
            if (generic_keyboard_create_devfs() < 0) {
                kpanic("Can't init pl050_keyboard in /dev");
            }
        }
    }
}

static void _pl050_keyboard_int_handler()
{
    uint32_t data = registers->data;
    generic_emit_key_set1(data);
}

static inline void _keyboard_send_cmd(uint8_t cmd)
{
    registers->data = cmd;
    while ((registers->stat) & (1 << 5)) { }
    int tmp = registers->data;
    ASSERT(tmp == 0xfa);
}

int pl050_keyboard_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    if (_pl050_map_itself(dev)) {
#ifdef DEBUG_PL050
        log_error("PL050 KBD: Can't map itself!");
#endif
        return -1;
    }

#ifdef DEBUG_PL050
    log("PL050 KBD: Turning on");
#endif
    registers->cr = 0x4 | 0x10;

    // Turning Scan Code Set 1
    _keyboard_send_cmd(0xF0);
    _keyboard_send_cmd(0x01);
    generic_keyboard_init();

    devtree_entry_t* devtree_entry = dev->device_desc.devtree.entry;
    ASSERT(devtree_entry->irq_lane > 0);
    irq_flags_t irqflags = irq_flags_from_devtree(devtree_entry->irq_flags);
    irq_register_handler(devtree_entry->irq_lane, devtree_entry->irq_priority, irqflags, _pl050_keyboard_int_handler, BOOT_CPU_MASK);
    return 0;
}

static driver_desc_t _pl050_keyboard_driver_info()
{
    driver_desc_t ms_desc = { 0 };
    ms_desc.type = DRIVER_INPUT_SYSTEMS_DEVICE;
    ms_desc.system_funcs.init_with_dev = pl050_keyboard_init;
    ms_desc.system_funcs.recieve_notification = pl050_keyboard_recieve_notification;
    ms_desc.functions[DRIVER_INPUT_SYSTEMS_ADD_DEVICE] = pl050_keyboard_init;
    ms_desc.functions[DRIVER_INPUT_SYSTEMS_GET_LAST_KEY] = 0;
    ms_desc.functions[DRIVER_INPUT_SYSTEMS_DISCARD_LAST_KEY] = 0;
    return ms_desc;
}

void pl050_keyboard_install()
{
    devman_register_driver(_pl050_keyboard_driver_info(), "pl050k");
}

devman_register_driver_installation(pl050_keyboard_install);