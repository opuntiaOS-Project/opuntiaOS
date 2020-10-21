/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Compositor.h"
#include "Screen.h"
#include "Window.h"
#include "WindowManager.h"
#include <libfoundation/EventLoop.h>
#include <libg/Context.h>

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
    LG::Context ctx(screen.write_bitmap());

    for (int i = 0; i < 1024 * 768; i++) {
        screen.write_bitmap().data()[i] = 0x00FeeeeF; // background
    }

    auto draw_window = [&](Window& window) {
        ctx.set_fill_color(LG::Color::Black);
        ctx.add_clip(window.bounds());
        window.frame().draw(ctx);
        ctx.draw({ window.content_x(), window.content_y() }, window.content_bitmap());
        ctx.reset_clip();
    };

    for (int win = 0; win < wm.windows().size(); win++) {
        draw_window(wm.windows()[win]);
    }

    // FIXME: Remove this
    if (wm.windows().size()) {
        wm.windows()[0].set_needs_display({0, 0, 400, 300});
    }

    ctx.set_fill_color(LG::Color::Green);
    ctx.fill(LG::Rect(wm.mouse_x(), wm.mouse_y(), wm.cursor_size(), wm.cursor_size()));

    screen.swap_buffers();
}