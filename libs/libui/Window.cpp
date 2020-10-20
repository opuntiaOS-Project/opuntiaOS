/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Window.h"
#include "App.h"
#include "Connection.h"
#include <syscalls.h>

namespace UI {

Window::Window()
    : Window(400, 300)
{
}

Window::Window(uint32_t width, uint32_t height)
    : m_bounds(0, 0, width, height)
    , m_buffer(size_t(width * height * 4))
    , m_bitmap()
{
    m_id = Connection::the().new_window(*this);
    m_bitmap = LG::PixelBitmap(m_buffer.data(), bounds().width(), bounds().height());
    App::the().set_window(this);
}

bool Window::set_title(const LG::String& title)
{
    SetTitleMessage msg(id(), title);
    return App::the().connection().send_async_message(msg);
}

void Window::receive_event(UniquePtr<LFoundation::Event> event)
{
    if (event->type() == Event::Type::MouseEvent) {
        if (m_superview) {
            MouseEvent& own_event = *(MouseEvent*)event.get();
            auto view = m_superview->hit_test({ own_event.x(), own_event.y() });
            view->receive_mouse_event(own_event);
        }
    }

    if (event->type() == Event::Type::DisplayEvent) {
        if (m_superview) {
            DisplayEvent& own_event = *(DisplayEvent*)event.get();
            m_superview->receive_display_event(own_event);
        }
    }
    // write(1, "Recieve mouse event\n", 20);
}

}