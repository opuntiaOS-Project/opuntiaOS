/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/Memory.h>
#include <libui/App.h>
#include <libui/Connection.h>
#include <libui/Window.h>

namespace UI {

Window::Window(const LG::Size& size, WindowType type)
    : m_bounds(0, 0, size.width(), size.height())
    , m_buffer(size_t(size.width() * size.height() * 4))
    , m_bitmap()
    , m_type(type)
{
    m_id = Connection::the().new_window(*this);
    m_menubar.set_host_window_id(m_id);
    m_bitmap = LG::PixelBitmap(m_buffer.data(), bounds().width(), bounds().height());
    App::the().set_window(this);
}

Window::Window(const LG::Size& size, const LG::string& icon_path)
    : m_bounds(0, 0, size.width(), size.height())
    , m_buffer(size_t(size.width() * size.height() * 4))
    , m_bitmap()
    , m_icon_path(icon_path)
{
    m_id = Connection::the().new_window(*this);
    m_menubar.set_host_window_id(m_id);
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
    if (bitmap().format() == LG::PixelBitmapFormat::RGBA) {
        // Set full bitmap as opaque, to mix colors correctly.
        fill_with_opaque(bounds());
    }

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

    if (event->type() == Event::Type::MouseWheelEvent) {
        if (m_superview) {
            MouseWheelEvent& own_event = *(MouseWheelEvent*)event.get();
            m_superview->receive_mouse_wheel_event(own_event);
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

            // If the window is in RGBA mode, we have to fill this rect
            // with opaque color before superview will mix it's color on
            // top of bitmap.
            if (bitmap().format() == LG::PixelBitmapFormat::RGBA) {
                fill_with_opaque(own_event.bounds());
            }

            m_superview->receive_display_event(own_event);
        }
    }

    if (event->type() == Event::Type::LayoutEvent) {
        if (m_superview) {
            LayoutEvent& own_event = *(LayoutEvent*)event.get();
            m_superview->receive_layout_event(own_event);
        }
    }

    if (event->type() == Event::Type::MenuBarActionEvent) {
        MenuBarActionEvent& own_event = *(MenuBarActionEvent*)event.get();
        if (own_event.item_id() < menubar().menu_items().size()) [[likely]] {
            menubar().menu_items()[own_event.item_id()].invoke();
        }
    }
}

void Window::fill_with_opaque(const LG::Rect& rect)
{
    const auto color = LG::Color(0, 0, 0, 0).u32();
    auto draw_bounds = rect;
    draw_bounds.intersect(bounds());
    if (draw_bounds.empty()) {
        return;
    }

    int min_x = draw_bounds.min_x();
    int min_y = draw_bounds.min_y();
    int max_x = draw_bounds.max_x();
    int max_y = draw_bounds.max_y();
    int len_x = max_x - min_x + 1;
    for (int y = min_y; y <= max_y; y++) {
        LFoundation::fast_set((uint32_t*)&m_bitmap[y][min_x], color, len_x);
    }
}

} // namespace UI