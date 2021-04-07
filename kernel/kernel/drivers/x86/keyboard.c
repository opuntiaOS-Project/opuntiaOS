/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/ringbuffer.h>
#include <drivers/x86/keyboard.h>
#include <io/tty/tty.h>
#include <libkern/libkern.h>
#include <platform/x86/idt.h>

static driver_desc_t _keyboard_driver_info();
static key_t _kbdriver_apply_modifiers(key_t key);

static void _kbdriver_notification(uint32_t msg, uint32_t param)
{
    if (msg == DM_NOTIFICATION_DEVFS_READY) {
        if (generic_keyboard_create_devfs() < 0) {
            kpanic("Can't init keyboard in devfs");
        }
    }
}

static driver_desc_t _keyboard_driver_info()
{
    driver_desc_t kbd_desc = { 0 };
    kbd_desc.type = DRIVER_INPUT_SYSTEMS_DEVICE;
    kbd_desc.auto_start = true;
    kbd_desc.is_device_driver = false;
    kbd_desc.is_device_needed = false;
    kbd_desc.is_driver_needed = false;
    kbd_desc.functions[DRIVER_NOTIFICATION] = _kbdriver_notification;
    kbd_desc.functions[DRIVER_INPUT_SYSTEMS_ADD_DEVICE] = kbdriver_run;
    kbd_desc.functions[DRIVER_INPUT_SYSTEMS_GET_LAST_KEY] = 0;
    kbd_desc.functions[DRIVER_INPUT_SYSTEMS_DISCARD_LAST_KEY] = 0;
    kbd_desc.pci_serve_class = 0xff;
    kbd_desc.pci_serve_subclass = 0xff;
    kbd_desc.pci_serve_vendor_id = 0x00;
    kbd_desc.pci_serve_device_id = 0x00;
    return kbd_desc;
}

bool kbdriver_install()
{
    driver_install(_keyboard_driver_info(), "kbd86");
    return true;
}

void kbdriver_run()
{
    set_irq_handler(IRQ1, keyboard_handler);
    generic_keyboard_init();
}

/* Keyboard interrupt handler */
void keyboard_handler()
{
    uint32_t scancode = (uint32_t)port_byte_in(0x60);
    generic_emit_key_set1(scancode);
}