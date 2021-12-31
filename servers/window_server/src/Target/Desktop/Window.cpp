/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Window.h"
#include "../../Managers/WindowManager.h"
#include <utility>

namespace WinServer::Desktop {

Window::Window(int connection_id, int id, CreateWindowMessage& msg)
    : BaseWindow(connection_id, id, msg)
    , m_frame(*this)
{
    m_bounds = LG::Rect(0, 0, msg.width() + frame().left_border_size() + frame().right_border_size(), msg.height() + frame().top_border_size() + frame().bottom_border_size());
    m_content_bounds = LG::Rect(m_frame.left_border_size(), m_frame.top_border_size(), msg.width(), msg.height());
    m_content_bitmap = LG::PixelBitmap(m_buffer.data(), content_bounds().width(), content_bounds().height());

    // Creating standard menubar directory entry.
    m_menubar_content.push_back(MenuDir(m_app_name, 0));
    m_menubar_content[0].add_item(PopupItem { PopupItem::InternalId, "Minimize others", [this](int) { WindowManager::the().minimize_windows([this](Window* win) { return win != this; }); } });
    m_menubar_content[0].add_item(PopupItem { PopupItem::InternalId, "Minimize", [this](int) { WindowManager::the().minimize_window(*this); } });
    m_menubar_content[0].add_item(PopupItem { PopupItem::InternalId, "Close", [this](int) { WindowManager::the().close_window(*this); } });
}

Window::Window(Window&& win)
    : BaseWindow(std::move(win))
    , m_frame(*this, std::move(win.m_frame.control_panel_buttons()), std::move(win.m_frame.window_control_buttons()))
    , m_corner_mask(std::move(win.m_corner_mask))
    , m_menubar_content(std::move(win.m_menubar_content))
{
}

void Window::make_frame()
{
    uint32_t x = m_bounds.min_x();
    uint32_t y = m_bounds.min_y();
    uint32_t content_width = m_content_bounds.width();
    uint32_t content_height = m_content_bounds.height();
    m_bounds = LG::Rect(x, y, content_width + frame().left_border_size() + frame().right_border_size(), content_height + frame().top_border_size() + frame().bottom_border_size());
    m_content_bounds = LG::Rect(x + m_frame.left_border_size(), y + m_frame.top_border_size(), content_width, content_height);
    m_frame.set_visible(true);
}

void Window::make_frameless()
{
    uint32_t x = m_bounds.min_x();
    uint32_t y = m_bounds.min_y();
    uint32_t content_width = m_content_bounds.width();
    uint32_t content_height = m_content_bounds.height();
    m_bounds = LG::Rect(x, y, content_width, content_height);
    m_content_bounds = LG::Rect(x, y, content_width, content_height);
    m_frame.set_visible(false);
}

void Window::recalc_bounds(const LG::Size& size)
{
    m_content_bounds.set_width(size.width());
    m_content_bounds.set_height(size.height());

    m_bounds.set_width(size.width() + frame().left_border_size() + frame().right_border_size());
    m_bounds.set_height(size.height() + frame().top_border_size() + frame().bottom_border_size());
}

void Window::did_size_change(const LG::Size& size)
{
    recalc_bounds(size);
}

void Window::on_style_change()
{
    WindowManager::the().on_window_style_change(*this);
}

void Window::on_menubar_change()
{
    WindowManager::the().on_window_menubar_change(*this);
}

} // namespace WinServer