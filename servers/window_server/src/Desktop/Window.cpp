/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Window.h"
#include "../WindowManager.h"
#include <utility>

namespace WinServer::Desktop {

Window::Window(int connection_id, int id, const CreateWindowMessage& msg)
    : BaseWindow(connection_id, id, msg)
    , m_frame(*this)
{
    m_bounds = LG::Rect(0, 0, msg.width() + frame().left_border_size() + frame().right_border_size(), msg.height() + frame().top_border_size() + frame().bottom_border_size());
    m_content_bounds = LG::Rect(m_frame.left_border_size(), m_frame.top_border_size(), msg.width(), msg.height());
    m_content_bitmap = LG::PixelBitmap(m_buffer.data(), content_bounds().width(), content_bounds().height());

    // Creating standard menubar directory entry.
    m_menubar_content.push_back(MenuDir("App", 0));
}

Window::Window(Window&& win)
    : BaseWindow(std::move(win))
    , m_frame(*this, std::move(win.m_frame.control_panel_buttons()), std::move(win.m_frame.window_control_buttons()))
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

} // namespace WinServer