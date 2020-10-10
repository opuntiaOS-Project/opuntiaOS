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
    char id = m_window.id() + '0';
    write(1, &id, 1);
}

void WindowFrame::draw(LG::PixelBitmap& bitmap)
{
    int x = m_window.bounds().x();
    int y = m_window.bounds().y();
    size_t width = m_window.bounds().width();
    size_t height = m_window.bounds().height();

    int right_x = x + width - right_border_size();
    int bottom_y = y + height - bottom_border_size();

    bitmap.draw(x, y, LG::Rect(0, 0, width, top_border_size()));
    bitmap.draw(x, y, LG::Rect(0, 0, left_border_size(), height));
    
    bitmap.draw(right_x, y, LG::Rect(0, 0, right_border_size(), height));
    bitmap.draw(x, bottom_y, LG::Rect(0, 0, width, bottom_border_size()));
}

void WindowFrame::receive_mouse_event(UniquePtr<MouseEvent> event)
{
}

const LG::Rect WindowFrame::bounds() const
{
    const auto& bounds = m_window.bounds();
    return LG::Rect(bounds.x(), bounds.y(), bounds.width(), top_border_size());
}