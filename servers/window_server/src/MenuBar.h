/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "MenuWidgets/MenuWidget.h"
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

    template <class T, class... Args>
    T& add_widget(Args&&... args)
    {
        T* widget = new T(args...);
        m_widgets.push_back(widget);
        return *widget;
    }

    [[gnu::always_inline]] inline void draw_widgets(LG::Context& ctx)
    {
        auto offset = ctx.draw_offset();
        size_t start_offset = MenuBar::width();

        for (int wind = m_widgets.size() - 1; wind >= 0; wind--) {
            start_offset -= m_widgets[wind]->width();
            ctx.set_draw_offset(LG::Point<int>(start_offset, 0));
            m_widgets[wind]->draw(ctx);
            start_offset -= 4;
        }

        ctx.set_draw_offset(offset);
    }

    [[gnu::always_inline]] inline void draw(LG::Context& ctx)
    {
        ctx.set_fill_color(m_background_color);
        ctx.mix({ 0, 0, MenuBar::width(), MenuBar::height() });

        draw_widgets(ctx);
    }

private:
    std::vector<MenuWidget*> m_widgets;
    LG::Color m_background_color;
};

} // namespace WinServer