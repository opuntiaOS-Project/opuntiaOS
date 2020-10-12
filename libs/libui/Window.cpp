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
    : m_bounds(0, 0, 400, 300)
    , m_buffer(size_t(400 * 300 * 4))
{
    m_id = Connection::the().new_window(*this);
    App::the().set_window(this);
}

Window::Window(uint32_t width, uint32_t height)
    : m_bounds(0, 0, width, height)
    , m_buffer(size_t(width * height * 4))
{
    m_id = Connection::the().new_window(*this);
    App::the().set_window(this);
}

// test
void Window::run()
{
    int n = bounds().height() * bounds().width();
    for (int i = 0; i < n; i++) {
        m_buffer[i] = 0x00ffffff;
    }
}

void Window::receive_event(UniquePtr<LFoundation::Event> event)
{
    if (m_superview) {
        UniquePtr<MouseEvent> mouse_event((MouseEvent*)event.release());
        auto view = m_superview->hit_test({ mouse_event->x(), mouse_event->y() });
        view->receive_mouse_event(move(mouse_event));
    }
    // write(1, "Recieve mouse event\n", 20);
}

}