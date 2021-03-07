/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libui/App.h>
#include <libui/Connection.h>
#include <libui/Window.h>
#include <syscalls.h>

namespace UI {

Window::Window()
    : Window(400, 300)
{
}

Window::Window(uint32_t width, uint32_t height, WindowType type)
    : m_bounds(0, 0, width, height)
    , m_buffer(size_t(width * height * 4))
    , m_bitmap()
    , m_type(type)
{
    m_id = Connection::the().new_window(*this);
    m_bitmap = LG::PixelBitmap(m_buffer.data(), bounds().width(), bounds().height());
    App::the().set_window(this);
}

Window::Window(uint32_t width, uint32_t height, const LG::string& icon_path)
    : m_bounds(0, 0, width, height)
    , m_buffer(size_t(width * height * 4))
    , m_bitmap()
    , m_icon_path(icon_path)
{
    m_id = Connection::the().new_window(*this);
    m_bitmap = LG::PixelBitmap(m_buffer.data(), bounds().width(), bounds().height());
    App::the().set_window(this);
}

bool Window::set_title(const LG::string& title)
{
    SetTitleMessage msg(Connection::the().key(), id(), title);
    return App::the().connection().send_async_message(msg);
}

bool Window::set_frame_style(const LG::Color& color)
{
    SetBarStyleMessage msg(Connection::the().key(), id(), color.u32(), 0);
    return App::the().connection().send_async_message(msg);
}

bool Window::did_format_change()
{
    SetBufferMessage msg(Connection::the().key(), id(), buffer().id(), bitmap().format());
    return App::the().connection().send_async_message(msg);
}

void Window::receive_event(std::unique_ptr<LFoundation::Event> event)
{
    if (event->type() == Event::Type::MouseEvent) {
        if (m_superview) {
            MouseEvent& own_event = *(MouseEvent*)event.get();
            m_superview->receive_mouse_move_event(own_event);
        }
    }

    if (event->type() == Event::Type::MouseActionEvent) {
        if (m_superview) {
            MouseActionEvent& own_event = *(MouseActionEvent*)event.get();
            auto* view = m_superview->hit_test({ (int)own_event.x(), (int)own_event.y() });
            view->receive_mouse_action_event(own_event);
        }
    }

    if (event->type() == Event::Type::MouseLeaveEvent) {
        if (m_superview) {
            MouseLeaveEvent& own_event = *(MouseLeaveEvent*)event.get();
            m_superview->receive_mouse_leave_event(own_event);
        }
    }

    if (event->type() == Event::Type::KeyUpEvent) {
        if (m_focused_view) {
            KeyUpEvent& own_event = *(KeyUpEvent*)event.get();
            m_focused_view->receive_keyup_event(own_event);
        }
    }

    if (event->type() == Event::Type::KeyDownEvent) {
        if (m_focused_view) {
            KeyDownEvent& own_event = *(KeyDownEvent*)event.get();
            m_focused_view->receive_keydown_event(own_event);
        }
    }

    if (event->type() == Event::Type::DisplayEvent) {
        if (m_superview) {
            DisplayEvent& own_event = *(DisplayEvent*)event.get();
            m_superview->receive_display_event(own_event);
        }
    }
}

} // namespace UI