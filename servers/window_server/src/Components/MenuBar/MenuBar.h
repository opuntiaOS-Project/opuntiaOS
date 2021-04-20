/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../CursorManager.h"
#include "../../Window.h"
#include "MenuItem.h"
#include "Widgets/BaseWidget.h"
#include <libfoundation/Logger.h>
#include <libg/Context.h>
#include <libg/PixelBitmap.h>
#include <libg/Point.h>
#include <vector>

namespace WinServer {

class MenuBar {
public:
    static MenuBar& the();
    MenuBar();

    static constexpr size_t height() { return 20; }
    static constexpr size_t width() { return 1024; }
    static constexpr size_t menubar_content_offset() { return 20; }

    LG::Rect& bounds() { return m_bounds; }
    const LG::Rect& bounds() const { return m_bounds; }

    template <class T, class... Args>
    T& add_widget(Args&&... args)
    {
        T* widget = new T(args...);
        m_widgets.push_back(widget);
        return *widget;
    }

    inline bool is_hovered() const { return m_hovered; }

    inline void draw_bar_items(LG::Context& ctx)
    {
        if (!m_menubar_content) {
            return;
        }

        auto offset = ctx.draw_offset();
        size_t start_offset = menubar_content_offset();
        auto& content = *m_menubar_content;

        for (int ind = 0; ind < content.size(); ind++) {
            start_offset += 4;
            ctx.set_draw_offset(LG::Point<int>(start_offset, 0));
            content[ind].draw(ctx);
            start_offset += content[ind].width();
        }

        ctx.set_draw_offset(offset);
    }

    inline void draw_widgets(LG::Context& ctx)
    {
        auto offset = ctx.draw_offset();
        size_t start_offset = MenuBar::width();

        for (int wind = m_widgets.size() - 1; wind >= 0; wind--) {
            start_offset -= 4;
            start_offset -= m_widgets[wind]->width();
            ctx.set_draw_offset(LG::Point<int>(start_offset, 0));
            m_widgets[wind]->draw(ctx);
        }

        ctx.set_draw_offset(offset);
    }

    inline void draw_logo(LG::Context& ctx)
    {
        ctx.draw({ 4, 4 }, m_logo);
    }

    [[gnu::always_inline]] inline void draw(LG::Context& ctx)
    {
        ctx.set_fill_color(m_background_color);
        ctx.mix({ 0, 0, MenuBar::width(), MenuBar::height() });

        draw_logo(ctx);
        draw_bar_items(ctx);
        draw_widgets(ctx);
    }

    inline void on_mouse_move(const CursorManager& cursor_manager)
    {
        m_hovered = true;
    }

    inline void on_mouse_leave(const CursorManager& cursor_manager)
    {
        m_hovered = false;
    }

    inline void on_mouse_status_change(const CursorManager& cursor_manager)
    {
        // Checking recievers
        int target_widget = find_widget(cursor_manager.x(), cursor_manager.y());
        if (target_widget >= 0) {
            widget_recieve_mouse_status_change(cursor_manager, (size_t)target_widget);
        }
    }

    inline std::vector<MenuDir>* menubar_content() const { return m_menubar_content; }

    inline void set_menubar_content(std::vector<MenuDir>* mc)
    {
        invalidate_menubar_panel();
        m_menubar_content = mc;
        invalidate_menubar_panel();
    }
    inline void set_menubar_content(std::vector<MenuDir>* mc, Compositor& compositor)
    {
        invalidate_menubar_panel(compositor);
        m_menubar_content = mc;
        invalidate_menubar_panel(compositor);
    }

private:
    // Widgets
    WidgetAnswer widget_recieve_mouse_status_change(const CursorManager& cursor_manager, size_t wind);
    inline size_t start_of_widget(size_t index)
    {
        size_t start_offset = MenuBar::width();
        for (int wind = m_widgets.size() - 1; wind >= (int)index; wind--) {
            start_offset -= 4;
            start_offset -= m_widgets[wind]->width();
        }
        return start_offset;
    }

    inline int find_widget(int x, int y)
    {
        size_t start_offset = MenuBar::width();
        for (int wind = m_widgets.size() - 1; wind >= 0; wind--) {
            start_offset -= 4;
            int end_offset = start_offset;
            start_offset -= m_widgets[wind]->width();
            if (start_offset <= x && x <= end_offset) {
                return wind;
            }
        }
        return -1;
    }

    // MenuBar Panel
    void invalidate_menubar_panel();
    void invalidate_menubar_panel(Compositor& compositor);
    static inline size_t menubar_panel_width(const std::vector<MenuDir>& items)
    {
        size_t start_offset = 0;
        for (int ind = 0; ind < items.size(); ind++) {
            start_offset += 4;
            start_offset += items[ind].width();
        }
        return start_offset;
    }

    inline size_t start_of_menubar_panel_item(size_t index)
    {
        if (!m_menubar_content) {
            return 0;
        }

        auto& content = *m_menubar_content;
        size_t start_offset = menubar_content_offset();
        for (int ind = 0; ind < index; ind++) {
            start_offset += 4;
            start_offset += content[ind].width();
        }
        return start_offset;
    }

    LG::Rect m_bounds;
    std::vector<MenuDir>* m_menubar_content { nullptr };
    std::vector<BaseWidget*> m_widgets;
    LG::Color m_background_color;
    LG::PixelBitmap m_logo;

    bool m_hovered { false };
};

} // namespace WinServer