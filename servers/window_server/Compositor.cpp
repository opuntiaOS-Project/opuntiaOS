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
    screen.swap_buffers();
}