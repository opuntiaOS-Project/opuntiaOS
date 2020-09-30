/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Compositor.h"
#include "Screen.h"
#include <libg/PixelBitmap.h>

static Compositor* s_the;

Compositor& Compositor::the()
{
    return *s_the;
}

Compositor::Compositor()
{
    s_the = this;
}

void Compositor::add_window(Window&& window)
{
    m_windows.push_back(move(window));
}

void Compositor::refresh()
{
    auto& screen = Screen::the();

    for (int i = 0; i < 1024 * 768; i++) {
        screen.write_bitmap().data()[i] = 0x00FeeeeF; // background
    }

    for (int win = 0; win < m_windows.size(); win++) {
        screen.write_bitmap().draw(m_windows[win].x(), m_windows[win].y(), m_windows[win].bitmap());
    }
    screen.swap_buffers();
}