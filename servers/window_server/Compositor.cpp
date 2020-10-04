/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Compositor.h"
#include "Screen.h"
#include "WindowManager.h"
#include <libfoundation/EventLoop.h>
#include <libg/PixelBitmap.h>

static Compositor* s_the;

Compositor& Compositor::the()
{
    return *s_the;
}

Compositor::Compositor()
{
    s_the = this;
    LFoundation::EventLoop::the().add(LFoundation::Timer([] {
        Compositor::the().refresh();
    }));
}

void Compositor::refresh()
{
    auto& screen = Screen::the();
    auto& wm = WindowManager::the();

    for (int i = 0; i < 1024 * 768; i++) {
        screen.write_bitmap().data()[i] = 0x00FeeeeF; // background
    }

    for (int win = 0; win < wm.windows().size(); win++) {
        screen.write_bitmap().draw(wm.windows()[win].x(), wm.windows()[win].y(), wm.windows()[win].bitmap());
    }

    int ox = wm.mouse_x();
    int oy = wm.mouse_y();
    for (int i = 0; i < wm.cursor_size(); i++) {
        for (int j = 0; j < wm.cursor_size(); j++) {
            if (!wm.is_cursor_pressed()) {
                screen.write_bitmap()[oy+i][ox+j] = 0x0000B4AB; // background
            } else {
                screen.write_bitmap()[oy+i][ox+j] = 0x00ABB400;
            }
        }
    }
    screen.swap_buffers();
}