/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "MenuBar.h"
#include "../../Compositor.h"
#include <libg/ImageLoaders/PNGLoader.h>

namespace WinServer {

static const uint32_t s_menu_button_glyph_data[12] = {
    0b000000000000,
    0b000000000000,
    0b011111111110,
    0b011111111110,
    0b000000000000,
    0b011111100000,
    0b011111100000,
    0b000000000000,
    0b011111111110,
    0b011111111110,
    0b000000000000,
    0b000000000000,
};

MenuBar* s_WinServer_MenuBar_the = nullptr;

MenuBar::MenuBar()
    : m_background_color(LG::Color::Opaque)
    , m_bounds(0, 0, Screen::the().bounds().width(), height())
    , m_popup(Popup::the())
    , m_logo(s_menu_button_glyph_data, 12, 12)
{
    s_WinServer_MenuBar_the = this;
    LG::PNG::PNGLoader loader;
}

void MenuBar::invalidate_widget(BaseWidget* wg)
{
    for (int i = 0; i < m_widgets.size(); i++) {
        if (m_widgets[i] == wg) {
            size_t widget_min_x = widget_start_offset(i);
            Compositor::the().invalidate(LG::Rect(widget_min_x, 0, m_widgets[i]->width(), height()));
            return;
        }
    }
}

MenuItemAnswer MenuBar::widget_recieve_mouse_status_change(const CursorManager& cursor_manager, size_t wind)
{
    if (wind >= m_widgets.size()) {
        return MenuItemAnswer::Bad;
    }

    MenuItemAnswer answer = MenuItemAnswer::Empty;
    size_t widget_min_x = widget_start_offset(wind);
    if (cursor_manager.pressed<CursorManager::Params::LeftButton>()) {
        answer = m_widgets[wind]->mouse_down(cursor_manager.x() - widget_min_x, cursor_manager.y());
    } else {
        answer = m_widgets[wind]->mouse_up();
    }

    if (answer & MenuItemAnswer::Bad) {
        return answer;
    }
    if (answer & MenuItemAnswer::InvalidateMe) {
        Compositor::the().invalidate(LG::Rect(widget_min_x, 0, m_widgets[wind]->width(), height()));
    }

    return answer;
}

MenuItemAnswer MenuBar::panel_item_recieve_mouse_status_change(const CursorManager& cursor_manager, size_t ind)
{
    if (!m_menubar_content) {
        return MenuItemAnswer::Bad;
    }

    auto& content = *m_menubar_content;
    if (ind >= content.size()) {
        return MenuItemAnswer::Bad;
    }

    MenuItemAnswer answer = MenuItemAnswer::Empty;
    size_t item_min_x = panel_item_start_offset(ind);
    if (cursor_manager.pressed<CursorManager::Params::LeftButton>()) {
        answer = content[ind].mouse_down(cursor_manager.x() - item_min_x, cursor_manager.y());
    } else {
        answer = content[ind].mouse_up();
    }

    if (answer & MenuItemAnswer::Bad) {
        return answer;
    }
    if (answer & MenuItemAnswer::InvalidateMe) {
        Compositor::the().invalidate(LG::Rect(item_min_x, 0, content[ind].width(), height()));
    }
    if (answer & MenuItemAnswer::PopupShow) {
        popup_will_be_shown(ind);
    }
    if (answer & MenuItemAnswer::PopupClose) {
        popup_will_be_closed();
    }

    return answer;
}

void MenuBar::invalidate_menubar_panel(Compositor& compositor)
{
    if (menubar_content()) {
        size_t inv_len = menubar_panel_width(*m_menubar_content);
        compositor.invalidate(LG::Rect(menubar_content_offset(), 0, inv_len, height()));
    }
}

void MenuBar::invalidate_menubar_panel()
{
    invalidate_menubar_panel(Compositor::the());
}

void MenuBar::set_style(StatusBarStyle ts)
{
    set_background_color(ts.color());

    if (ts.dark_text()) {
        m_text_color = LG::Color::DarkSystemText;
    } else {
        m_text_color = LG::Color::LightSystemText;
    }
}

} // namespace WinServer