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
    : m_width(400)
    , m_height(300)
    , m_buffer(size_t(m_width * m_height * 4))
{
    m_id = Connection::the().new_window(*this);
    App::the().set_window(this);
}

Window::Window(uint32_t width, uint32_t height)
    : m_width(width)
    , m_height(height)
    , m_buffer(size_t(m_width * m_height * 4))
{
    m_id = Connection::the().new_window(*this);
    App::the().set_window(this);
}

// test
void Window::run()
{
    for (int i = 0; i < m_width * m_height; i++) {
        m_buffer[i] = 0x00ffffff;
    }
}

void Window::receive_event(UniquePtr<LFoundation::Event> event)
{
    // write(1, "Recieve mouse event\n", 20);
}

}