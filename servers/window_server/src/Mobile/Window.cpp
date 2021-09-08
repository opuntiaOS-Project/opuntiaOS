/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Window.h"
#include "../WindowManager.h"
#include <utility>

namespace WinServer::Mobile {

Window::Window(int connection_id, int id, const CreateWindowMessage& msg)
    : BaseWindow(connection_id, id, msg)
{
    m_bounds = LG::Rect(0, 0, msg.width(), msg.height());
    m_content_bounds = LG::Rect(0, 0, msg.width(), msg.height());
    m_content_bitmap = LG::PixelBitmap(m_buffer.data(), content_bounds().width(), content_bounds().height());
}

Window::Window(Window&& win)
    : BaseWindow(std::move(win))
{
}

void Window::on_style_change()
{
    WindowManager::the().on_window_style_change(*this);
}

} // namespace WinServer