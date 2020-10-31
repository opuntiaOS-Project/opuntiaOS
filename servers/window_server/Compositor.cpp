/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Compositor.h"
#include "CursorManager.h"
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
    : m_cursor_manager(CursorManager::the())
{
    s_the = this;
    invalidate(Screen::the().bounds());
    LFoundation::EventLoop::the().add(LFoundation::Timer([] {
        Compositor::the().refresh();
    }));
}

void Compositor::refresh()
{
    if (m_invalidated_areas.size() == 0 && m_prev_invalidated_areas.size() == 0) {
        return;
    }

    auto invalidated_areas = move(m_invalidated_areas);
    auto prev_invalidated_areas = move(m_prev_invalidated_areas);
    auto& screen = Screen::the();
    auto& wm = WindowManager::the();
    LG::Context ctx(screen.write_bitmap());

    auto is_window_area_invalidated = [&](const Vector<LG::Rect>& areas, const LG::Rect& area) -> bool {
        for (int i = 0; i < areas.size(); i++) {
            if (area.intersects(areas[i])) {
                return true;
            }
        }
        return false;
    };

    auto draw_wallpaper_for_area = [&](const LG::Rect& area) {
        ctx.add_clip(area);
        ctx.set_fill_color(LG::Color(0x00FeeeeF));
        ctx.fill(LG::Rect(0, 0, 1024, 768));
        ctx.reset_clip();
    };

    auto draw_window = [&](Window& window, const LG::Rect& area) {
        ctx.set_fill_color(LG::Color::Black);
        ctx.add_clip(area);
        ctx.add_clip(window.bounds());
        window.frame().draw(ctx);
        ctx.draw(window.content_bounds().origin(), window.content_bitmap());
        ctx.reset_clip();
    };

    auto process_invalid_areas = [&](const Vector<LG::Rect>& areas) {
        for (int i = 0; i < areas.size(); i++) {
            draw_wallpaper_for_area(areas[i]);
        }
        for (int win = 0; win < wm.windows().size(); win++) {
            auto& window = wm.windows()[win];
            if (is_window_area_invalidated(areas, window.bounds())) {
                for (int i = 0; i < areas.size(); i++) {
                    draw_window(window, areas[i]);
                }
            }
        }
    };

    process_invalid_areas(prev_invalidated_areas);
    process_invalid_areas(invalidated_areas);

    ctx.draw(m_cursor_manager.draw_position(wm.mouse_x(), wm.mouse_y()), m_cursor_manager.current_cursor());

    m_prev_invalidated_areas = move(invalidated_areas);
    screen.swap_buffers();
}