/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Compositor.h"
#include "Components/Base/BaseWindow.h"
#include "Components/ControlBar/ControlBar.h"
#include "Components/MenuBar/MenuBar.h"
#include "Components/Popup/Popup.h"
#include "CursorManager.h"
#include "ResourceManager.h"
#include "Screen.h"
#include "WindowManager.h"
#include <libfoundation/EventLoop.h>
#include <libfoundation/Memory.h>
#include <libg/Context.h>

namespace WinServer {

static Compositor* s_the;

Compositor& Compositor::the()
{
    return *s_the;
}

Compositor::Compositor()
    : m_cursor_manager(CursorManager::the())
    , m_resource_manager(ResourceManager::the())
    , m_popup(Popup::the())
    , m_menu_bar(MenuBar::the())
#ifdef TARGET_MOBILE
    , m_control_bar(ControlBar::the())
#endif // TARGET_MOBILE
{
    s_the = this;
    invalidate(Screen::the().bounds());
    LFoundation::EventLoop::the().add(LFoundation::Timer([] {
        Compositor::the().refresh();
    },
        1000 / 60, LFoundation::Timer::Repeat));
}

void Compositor::copy_changes_to_second_buffer(const std::vector<LG::Rect>& areas)
{
    auto& screen = Screen::the();

    for (int i = 0; i < areas.size(); i++) {
        auto bounds = areas[i].intersection(screen.bounds());
        auto* buf1_ptr = reinterpret_cast<uint32_t*>(&screen.display_bitmap()[bounds.min_y()][bounds.min_x()]);
        auto* buf2_ptr = reinterpret_cast<uint32_t*>(&screen.write_bitmap()[bounds.min_y()][bounds.min_x()]);
        for (int j = 0; j < bounds.height(); j++) {
            LFoundation::fast_copy(buf2_ptr, buf1_ptr, bounds.width());
            buf1_ptr += screen.width();
            buf2_ptr += screen.width();
        }
    }
}

[[gnu::flatten]] void Compositor::refresh()
{
    if (m_invalidated_areas.size() == 0) {
        return;
    }

    auto& screen = Screen::the();
    auto& wm = WindowManager::the();
    auto invalidated_areas = std::move(m_invalidated_areas);
    LG::Context ctx(screen.write_bitmap());

    auto is_window_area_invalidated = [&](const std::vector<LG::Rect>& areas, const LG::Rect& area) -> bool {
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

#ifdef TARGET_DESKTOP
    auto draw_window = [&](Desktop::Window& window, const LG::Rect& area) {
        ctx.add_clip(area);
        ctx.add_clip(window.bounds());
        window.frame().draw(ctx);
        ctx.draw_rounded(window.content_bounds().origin(), window.content_bitmap(), window.corner_mask());
        ctx.reset_clip();
    };
#elif TARGET_MOBILE
    auto draw_window = [&](Mobile::Window& window, const LG::Rect& area) {
        ctx.add_clip(area);
        ctx.add_clip(window.bounds());
        ctx.draw(window.content_bounds().origin(), window.content_bitmap());
        ctx.reset_clip();
    };
#endif // TARGET_DESKTOP

#ifdef TARGET_DESKTOP
    for (int i = 0; i < invalidated_areas.size(); i++) {
        draw_wallpaper_for_area(invalidated_areas[i]);
    }
#elif TARGET_MOBILE
    // Draw wallpaper only in case when WM contains only homescreen app.
    if (wm.windows().size() <= 1) {
        for (int i = 0; i < invalidated_areas.size(); i++) {
            draw_wallpaper_for_area(invalidated_areas[i]);
        }
    }
#endif // TARGET_DESKTOP

    auto& windows = wm.windows();
#ifdef TARGET_DESKTOP
    for (auto it = windows.rbegin(); it != windows.rend(); it++) {
        auto& window = *(*it);
        if (window.visible() && is_window_area_invalidated(invalidated_areas, window.bounds())) {
            for (int i = 0; i < invalidated_areas.size(); i++) {
                draw_window(window, invalidated_areas[i]);
            }
        }
    }
#elif TARGET_MOBILE
    // Draw wallpaper only in case when WM contains homescreen app.
    if (windows.begin() != windows.end()) {
        auto& window = *(*windows.begin());
        if (is_window_area_invalidated(invalidated_areas, window.bounds())) {
            for (int i = 0; i < invalidated_areas.size(); i++) {
                draw_window(window, invalidated_areas[i]);
            }
        }
    }
#endif // TARGET_DESKTOP

    if (m_popup.visible()) {
        for (int i = 0; i < invalidated_areas.size(); i++) {
            ctx.add_clip(invalidated_areas[i]);
            m_popup.draw(ctx);
            ctx.reset_clip();
        }
    }

    for (int i = 0; i < invalidated_areas.size(); i++) {
        ctx.add_clip(invalidated_areas[i]);
        m_menu_bar.draw(ctx);
        ctx.reset_clip();
    }

#ifdef TARGET_MOBILE
    for (int i = 0; i < invalidated_areas.size(); i++) {
        ctx.add_clip(invalidated_areas[i]);
        m_control_bar.draw(ctx);
        ctx.reset_clip();
    }
#endif // TARGET_MOBILE

    auto mouse_draw_position = m_cursor_manager.draw_position();
    auto& current_mouse_bitmap = m_cursor_manager.current_cursor();
    for (int i = 0; i < invalidated_areas.size(); i++) {
        ctx.add_clip(invalidated_areas[i]);
        ctx.draw(mouse_draw_position, current_mouse_bitmap);
        ctx.reset_clip();
    }

    screen.swap_buffers();
    copy_changes_to_second_buffer(invalidated_areas);
}

} // namespace WinServer