/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/ringbuffer.h>
#include <drivers/aarch32/pl050.h>
#include <drivers/devtree.h>
#include <drivers/generic/mouse.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <platform/aarch32/interrupts.h>
#include <tasking/tasking.h>

// #define MOUSE_DRIVER_DEBUG
#ifdef MOUSE_DRIVER_DEBUG
#define DEBUG_PL050
#endif

static ringbuffer_t mouse_buffer;
static kmemzone_t mapped_zone;
static volatile pl050_registers_t* registers;

static inline uintptr_t _pl050_mmio_paddr(devtree_entry_t* device)
{
    if (!device) {
        kpanic("PL050 MOUSE: Can't find device in the tree.");
    }

    return (uintptr_t)device->paddr;
}

static inline int _pl050_map_itself(device_t* device)
{
    uintptr_t mmio_paddr = _pl050_mmio_paddr(device->device_desc.devtree.entry);

    mapped_zone = kmemzone_new(sizeof(pl050_registers_t));
    vmm_map_page(mapped_zone.start, mmio_paddr, PAGE_DEVICE);
    registers = (pl050_registers_t*)mapped_zone.ptr;
    return 0;
}

static bool _mouse_can_read(dentry_t* dentry, size_t start)
{
    return ringbuffer_space_to_read(&mouse_buffer) >= 1;
}

static int _mouse_read(dentry_t* dentry, uint8_t* buf, size_t start, size_t len)
{
    size_t leno = ringbuffer_space_to_read(&mouse_buffer);
    if (leno > len) {
        leno = len;
    }
    int res = ringbuffer_read(&mouse_buffer, buf, leno);
    return leno;
}
static void pl050_mouse_recieve_notification(uint32_t msg, uint32_t param)
{
    if (msg == DEVMAN_NOTIFICATION_DEVFS_READY) {
        dentry_t* mp;
        if (vfs_resolve_path("/dev", &mp) < 0) {
            kpanic("Can't init pl050_mouse in /dev");
        }

        file_ops_t fops = { 0 };
        fops.can_read = _mouse_can_read;
        fops.read = _mouse_read;
        devfs_inode_t* res = devfs_register(mp, MKDEV(10, 1), "mouse", 5, 0400, &fops);

        dentry_put(mp);
    }
}

static void _mouse_send_cmd_and_data(uint8_t cmd, uint8_t data)
{
    registers->data = cmd;
    while ((registers->stat) & (1 << 5)) { }
    int tmp = registers->data;
    ASSERT(tmp == 0xfa);
    registers->data = data;
    int tmp2 = registers->data;
    ASSERT(tmp2 == 0xfa);
}

static inline void _mouse_send_cmd(uint8_t cmd)
{
    registers->data = cmd;
    while ((registers->stat) & (1 << 5)) { }
    int tmp = registers->data;
    ASSERT(tmp == 0xfa);
}

#define KMIIR_RXINTR (1 << 0)
#define RXFULL (1 << 4)

static inline int16_t int8_to_int16_t_safe_convert(int8_t x)
{
    return x < 128 ? x : x - 256;
}

static void _pl050_mouse_int_handler()
{
    uint32_t statusrx = registers->stat;
    uint32_t status = registers->ir;
    uint32_t buffer[15];
    int indx = 0;

    if (!(status & KMIIR_RXINTR)) {
        return;
    }

    while (status & KMIIR_RXINTR) {
        buffer[indx++] = registers->data;
        status = registers->ir;
    }

    uint8_t resp = buffer[0];
    uint8_t xm = buffer[1];
    uint8_t ym = buffer[2];
    int16_t wheel = int8_to_int16_t_safe_convert(buffer[3]);

    uint8_t y_overflow = (resp >> 7) & 1;
    uint8_t x_overflow = (resp >> 6) & 1;
    uint8_t y_sign = (resp >> 5) & 1;
    uint8_t x_sign = (resp >> 4) & 1;

    mouse_packet_t packet;
    packet.x_offset = xm;
    packet.y_offset = ym;
    packet.button_states = resp & 0b111;
    packet.wheel_data = wheel;

    if (packet.x_offset && x_sign) {
        packet.x_offset -= 0x100;
    }
    if (packet.y_offset && y_sign) {
        packet.y_offset -= 0x100;
    }
    if (x_overflow || y_overflow) {
        packet.x_offset = 0;
        packet.y_offset = 0;
    }

    ringbuffer_write(&mouse_buffer, (uint8_t*)&packet, sizeof(mouse_packet_t));

#ifdef MOUSE_DRIVER_DEBUG
    log("%x ", packet.button_states);
    if (packet.x_offset < 0) {
        log("-%d ", -packet.x_offset);
    } else {
        log("%d ", packet.x_offset);
    }
    if (packet.y_offset < 0) {
        log("-%d\n", -packet.y_offset);
    } else {
        log("%d\n", packet.y_offset);
    }
#endif /* MOUSE_DRIVER_DEBUG */
}

int pl050_mouse_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }

    if (_pl050_map_itself(dev)) {
#ifdef DEBUG_PL050
        log_error("PL050 MOUSE: Can't map itself!");
#endif
        return -1;
    }

#ifdef DEBUG_PL050
    log("PL050 MOUSE: Turning on");
#endif
    registers->cr = 0x4 | 0x10;
    _mouse_send_cmd(0xF6);
    _mouse_send_cmd(0xF4);
    _mouse_send_cmd_and_data(0xF3, 200);
    _mouse_send_cmd_and_data(0xF3, 100);
    _mouse_send_cmd_and_data(0xF3, 80);
    irq_register_handler(PL050_MOUSE_IRQ_LINE, 0, 0, _pl050_mouse_int_handler, BOOT_CPU_MASK);
    mouse_buffer = ringbuffer_create_std();
    return 0;
}

static driver_desc_t _pl050_mouse_driver_info()
{
    driver_desc_t ms_desc = { 0 };
    ms_desc.type = DRIVER_INPUT_SYSTEMS_DEVICE;
    ms_desc.system_funcs.init_with_dev = pl050_mouse_init;
    ms_desc.system_funcs.recieve_notification = pl050_mouse_recieve_notification;
    ms_desc.functions[DRIVER_INPUT_SYSTEMS_ADD_DEVICE] = pl050_mouse_init;
    ms_desc.functions[DRIVER_INPUT_SYSTEMS_GET_LAST_KEY] = 0;
    ms_desc.functions[DRIVER_INPUT_SYSTEMS_DISCARD_LAST_KEY] = 0;
    return ms_desc;
}

void pl050_mouse_install()
{
    devman_register_driver(_pl050_mouse_driver_info(), "pl050m");
}

devman_register_driver_installation(pl050_mouse_install);