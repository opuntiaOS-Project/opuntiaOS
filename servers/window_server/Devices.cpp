/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Devices.h"

static Devices* s_the;

Devices::Devices()
{
    s_the = this;
    m_mouse_fd = open("/dev/mouse", O_RDONLY);
    if (!m_mouse_fd) {
        write(1, "can't open mouse", 16);
    }
 
    LFoundation::EventLoop::the().add(
        m_mouse_fd, [] {
            Devices::the().pump_mouse();
        },
        nullptr);
}

Devices& Devices::the()
{
    return *s_the;
}