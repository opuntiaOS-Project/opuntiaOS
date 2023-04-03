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

int mouse_run();

static void _mouse_recieve_notification(uintptr_t msg, uintptr_t param)
{
    if (msg == DEVMAN_NOTIFICATION_DEVFS_READY) {
        generic_mouse_create_devfs();
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
    while ((port_read8(0x64) & 1) == 0) { }
}

static inline void _mouse_wait_out()
{
    while ((port_read8(0x64) & 2) == 1) { }
}

static inline void _mouse_wait_then_write(uint16_t port, uint8_t data)
{
    _mouse_wait_out();
    port_write8(port, data);
}

static inline uint8_t _mouse_wait_then_read(uint16_t port)
{
    _mouse_wait_in();
    return port_read8(port);
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

void mouse_int_handler(irq_line_t il)
{
    uint8_t status = port_read8(0x64);
    if ((status & 0x1) == 0 || (status & 0x20) != 0x20) {
        return;
    }
    uint8_t resp = port_read8(0x60);
    uint8_t xm = port_read8(0x60);
    uint8_t ym = port_read8(0x60);
    int8_t wheel = port_read8(0x60);

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

    generic_mouse_send_packet(&packet);

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
    irq_register_handler(irqline_from_id(12), 0, 0, mouse_int_handler, BOOT_CPU_MASK);

    generic_mouse_init();
    return 0;
}

void mouse_install()
{
    devman_register_driver(_mouse_driver_info(), "mouse86");
}
devman_register_driver_installation(mouse_install);