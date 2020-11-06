/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Devices.h"
#include <std/Dbg.h>

static Devices* s_the;

Devices::Devices()
{
    s_the = this;
    m_mouse_fd = open("/dev/mouse", O_RDONLY);
    if (!m_mouse_fd) {
        Dbg() << "Can't open mouse\n";
    }

    m_keyboard_fd = open("/dev/kbd", O_RDONLY);
    if (!m_keyboard_fd) {
        Dbg() << "Can't open keyboard\n";
    }

    LFoundation::EventLoop::the().add(
        m_mouse_fd, [] {
            Devices::the().pump_mouse();
        },
        nullptr);

    LFoundation::EventLoop::the().add(
        m_keyboard_fd, [] {
            Devices::the().pump_keyboard();
        },
        nullptr);
}

Devices& Devices::the()
{
    return *s_the;
}