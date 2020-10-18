/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Window.h"
#include <std/Utility.h>

Window::Window(int id, const CreateWindowMessage& msg)
    : m_id(id)
    , m_buffer(msg.buffer_id())
    , m_frame(*this)
    , m_content_bitmap()
    , m_bounds(0, 0, 0, 0)
    , m_content_bounds(0, 0, 0, 0)
{
    m_bounds = LG::Rect(0, 0, msg.width() + frame().left_border_size() + frame().right_border_size(), msg.height() + frame().top_border_size() + frame().bottom_border_size());
    m_content_bounds = LG::Rect(m_frame.left_border_size(), m_frame.top_border_size(), msg.width(), msg.height());
    m_content_bitmap = LG::PixelBitmap(m_buffer.data(), content_bounds().width(), content_bounds().height());
}

Window::Window(Window&& win)
    : m_id(win.m_id)
    , m_buffer(win.m_buffer)
    , m_frame(*this, move(win.m_frame.control_panel_buttons()), move(win.m_frame.window_control_buttons()))
    , m_content_bitmap(move(win.m_content_bitmap))
    , m_bounds(win.m_bounds)
    , m_content_bounds(win.m_content_bounds)
{
}

void Window::set_buffer(int buffer_id)
{
    m_buffer.open(buffer_id);
}