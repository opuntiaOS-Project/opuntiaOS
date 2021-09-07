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

MenuBar* s_WinServer_MenuBar_the = nullptr;

MenuBar::MenuBar()
    : m_background_color(LG::Color::Opaque)
    , m_bounds(0, 0, Screen::the().bounds().width(), height())
    , m_popup(Popup::the())
{
    s_WinServer_MenuBar_the = this;
    LG::PNG::PNGLoader loader;
    m_logo = loader.load_from_file("/res/system/menu_12.png");
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

} // namespace WinServer