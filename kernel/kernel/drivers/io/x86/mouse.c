/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/ringbuffer.h>
#include <drivers/driver_manager.h>
#include <drivers/io/x86/mouse.h>
#include <drivers/irq/irq_api.h>
#include <fs/devfs/devfs.h>
#include <libkern/kassert.h>
#include <libkern/log.h>
#include <libkern/types.h>
#include <platform/x86/idt.h>
#include <platform/x86/port.h>

// #define MOUSE_DRIVER_DEBUG

static ringbuffer_t mouse_buffer;

int mouse_run();

static bool _mouse_can_read(file_t* file, size_t start)
{
    return ringbuffer_space_to_read(&mouse_buffer) >= 1;
}

static int _mouse_read(file_t* file, void __user* buf, size_t start, size_t len)
{
    size_t leno = ringbuffer_space_to_read(&mouse_buffer);
    if (leno > len) {
        leno = len;
    }
    int res = ringbuffer_read_user(&mouse_buffer, buf, leno);
    return leno;
}

static void _mouse_recieve_notification(uintptr_t msg, uintptr_t param)
{
    if (msg == DEVMAN_NOTIFICATION_DEVFS_READY) {
        path_t vfspth;
        if (vfs_resolve_path("/dev", &vfspth) < 0) {
            kpanic("Can't init mouse in /dev");
        }

        file_ops_t fops = { 0 };
        fops.can_read = _mouse_can_read;
        fops.read = _mouse_read;
        devfs_inode_t* res = devfs_register(&vfspth, MKDEV(10, 1), "mouse", 5, S_IFCHR | 0400, &fops);

        path_put(&vfspth);
    }
}

static driver_desc_t _mouse_driver_info()
{
    driver_desc_t desc = { 0 };
    desc.type = DRIVER_INPUT_SYSTEMS_DEVICE;
    desc.flags = DRIVER_DESC_FLAG_START;
    desc.system_funcs.on_start = mouse_run;
    desc.system_funcs.recieve_notification = _mouse_recieve_notification;
    desc.functions[DRIVER_INPUT_SYSTEMS_ADD_DEVICE] = mouse_run;
    desc.functions[DRIVER_INPUT_SYSTEMS_GET_LAST_KEY] = 0;
    desc.functions[DRIVER_INPUT_SYSTEMS_DISCARD_LAST_KEY] = 0;
    return desc;
}

static inline void _mouse_wait_in()
{
    while ((port_8bit_in(0x64) & 1) == 0) { }
}

static inline void _mouse_wait_out()
{
    while ((port_8bit_in(0x64) & 2) == 1) { }
}

static inline void _mouse_wait_then_write(uint16_t port, uint8_t data)
{
    _mouse_wait_out();
    port_8bit_out(port, data);
}

static inline uint8_t _mouse_wait_then_read(uint16_t port)
{
    _mouse_wait_in();
    return port_8bit_in(port);
}

static inline void _mouse_send_cmd(uint8_t cmd)
{
    _mouse_wait_then_write(0x64, 0xD4);
    _mouse_wait_then_write(0x60, cmd);
    ASSERT(_mouse_wait_then_read(0x60) == 0xfa);
}

static void _mouse_send_cmd_and_data(uint8_t cmd, uint8_t data)
{
    _mouse_wait_then_write(0x64, 0xD4);
    _mouse_wait_then_write(0x60, cmd);
    ASSERT(_mouse_wait_then_read(0x60) == 0xfa);
    _mouse_wait_then_write(0x64, 0xD4);
    _mouse_wait_then_write(0x60, data);
    ASSERT(_mouse_wait_then_read(0x60) == 0xfa);
}

static inline void _mouse_enable_aux()
{
    _mouse_wait_then_write(0x64, 0x20);
    uint8_t res = _mouse_wait_then_read(0x60);
    res |= 0b10;
    res &= 0b11011111;
    _mouse_wait_then_write(0x64, 0x60);
    _mouse_wait_then_write(0x60, res);
}

void mouse_handler()
{
    uint8_t status = port_8bit_in(0x64);
    if ((status & 0x1) == 0 || (status & 0x20) != 0x20) {
        return;
    }
    uint8_t resp = port_8bit_in(0x60);
    uint8_t xm = port_8bit_in(0x60);
    uint8_t ym = port_8bit_in(0x60);
    int8_t wheel = port_8bit_in(0x60);

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
    log("%x", packet.button_states);
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

int mouse_run()
{
    _mouse_wait_then_write(0x64, 0xa8);
    _mouse_enable_aux();
    _mouse_send_cmd(0xF6);
    _mouse_send_cmd(0xF4);
    _mouse_send_cmd_and_data(0xF3, 200);
    _mouse_send_cmd_and_data(0xF3, 100);
    _mouse_send_cmd_and_data(0xF3, 80);
    irq_register_handler(irqline_from_id(12), 0, 0, mouse_handler, BOOT_CPU_MASK);

    mouse_buffer = ringbuffer_create_std();
    return 0;
}

void mouse_install()
{
    devman_register_driver(_mouse_driver_info(), "mouse86");
}
devman_register_driver_installation(mouse_install);