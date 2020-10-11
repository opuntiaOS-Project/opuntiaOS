/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "WindowFrame.h"
#include "Window.h"
#include <libg/Rect.h>
#include <std/Utility.h>
#include <syscalls.h>

WindowFrame::WindowFrame(Window& window)
    : m_window(window)
{
}

void WindowFrame::draw(LG::Context& ctx)
{
    int x = m_window.bounds().min_x();
    int y = m_window.bounds().min_y();
    size_t width = m_window.bounds().width();
    size_t height = m_window.bounds().height();

    int right_x = x + width - right_border_size();
    int bottom_y = y + height - bottom_border_size();

    ctx.fill(LG::Rect(x, y, width, top_border_size()));
    ctx.fill(LG::Rect(x, y, left_border_size(), height));
    
    ctx.fill(LG::Rect(right_x, y, right_border_size(), height));
    ctx.fill(LG::Rect(x, bottom_y, width, bottom_border_size()));
}

void WindowFrame::receive_mouse_event(UniquePtr<MouseEvent> event)
{
}

const LG::Rect WindowFrame::bounds() const
{
    const auto& bounds = m_window.bounds();
    return LG::Rect(bounds.min_x(), bounds.min_y(), bounds.width(), top_border_size());
}