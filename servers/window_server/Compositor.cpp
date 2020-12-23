/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Compositor.h"
#include "CursorManager.h"
#include "ResourceManager.h"
#include "Screen.h"
#include "Window.h"
#include "WindowManager.h"
#include <libfoundation/EventLoop.h>
#include <libg/Context.h>
#include <std/Memory.h>

static Compositor* s_the;

Compositor& Compositor::the()
{
    return *s_the;
}

Compositor::Compositor()
    : m_cursor_manager(CursorManager::the())
    , m_resource_manager(ResourceManager::the())
{
    s_the = this;
    invalidate(Screen::the().bounds());
    LFoundation::EventLoop::the().add(LFoundation::Timer([] {
        Compositor::the().refresh();
    }));
}

void Compositor::copy_changes_to_second_buffer(const Vector<LG::Rect>& areas)
{
    auto& screen = Screen::the();

    for (int i = 0; i < areas.size(); i++) {
        auto bounds = areas[i].intersection(screen.bounds());
        uint32_t* buf1_ptr = (uint32_t*)&screen.display_bitmap()[bounds.min_y()][bounds.min_x()];
        uint32_t* buf2_ptr = (uint32_t*)&screen.write_bitmap()[bounds.min_y()][bounds.min_x()];
        for (int j = 0; j < bounds.height(); j++) {
            fast_copy(buf2_ptr, buf1_ptr, bounds.width());
            buf1_ptr += screen.width();
            buf2_ptr += screen.width();
        }
    }
}

void Compositor::refresh()
{
    if (m_invalidated_areas.size() == 0) {
        return;
    }

    auto& screen = Screen::the();
    auto& wm = WindowManager::the();
    auto invalidated_areas = move(m_invalidated_areas);
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
        ctx.draw({ 0, 0 }, m_resource_manager.background());
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

    for (int i = 0; i < invalidated_areas.size(); i++) {
        draw_wallpaper_for_area(invalidated_areas[i]);
    }

    auto* window_ptr = wm.windows().tail();
    while (window_ptr) {
        auto& window = *window_ptr;
        if (is_window_area_invalidated(invalidated_areas, window.bounds())) {
            for (int i = 0; i < invalidated_areas.size(); i++) {
                draw_window(window, invalidated_areas[i]);
            }
        }
        window_ptr = window_ptr->prev();
    }

    auto mouse_draw_position = m_cursor_manager.draw_position(wm.mouse_x(), wm.mouse_y());
    auto& current_mouse_bitmap = m_cursor_manager.current_cursor();
    for (int i = 0; i < invalidated_areas.size(); i++) {
        ctx.add_clip(invalidated_areas[i]);
        ctx.draw(mouse_draw_position, current_mouse_bitmap);
        ctx.reset_clip();
    }

    screen.swap_buffers();
    copy_changes_to_second_buffer(invalidated_areas);
}