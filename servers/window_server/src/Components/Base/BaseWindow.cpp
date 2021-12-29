/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "BaseWindow.h"
#include "../../Managers/WindowManager.h"
#include <utility>

namespace WinServer {

BaseWindow::BaseWindow(int connection_id, int id, CreateWindowMessage& msg)
    : m_id(id)
    , m_connection_id(connection_id)
    , m_type((WindowType)msg.type())
    , m_buffer(msg.buffer_id())
    , m_content_bitmap()
    , m_bounds(0, 0, 0, 0)
    , m_content_bounds(0, 0, 0, 0)
    , m_app_name(msg.title().string())
    , m_bundle_id(msg.bundle_id().string())
{
}

BaseWindow::BaseWindow(BaseWindow&& win)
    : m_id(win.m_id)
    , m_connection_id(win.m_connection_id)
    , m_buffer(win.m_buffer)
    , m_content_bitmap(std::move(win.m_content_bitmap))
    , m_bounds(win.m_bounds)
    , m_content_bounds(win.m_content_bounds)
    , m_app_name(std::move(win.m_app_name))
    , m_icon_path(std::move(win.m_icon_path))
    , m_bundle_id(std::move(win.m_bundle_id))
{
}

void BaseWindow::set_buffer(int buffer_id, LG::Size sz, LG::PixelBitmapFormat fmt)
{
    m_buffer.open(buffer_id);
    m_content_bitmap = LG::PixelBitmap(m_buffer.data(), sz.width(), sz.height());
    m_content_bitmap.set_format(fmt);
}

} // namespace WinServer