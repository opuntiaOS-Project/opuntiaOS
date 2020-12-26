/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "WindowManager.h"
#include "CursorManager.h"
#include "Screen.h"
#include <libfoundation/KeyboardMapping.h>

static WindowManager* s_the;

WindowManager& WindowManager::the()
{
    return *s_the;
}

WindowManager::WindowManager()
    : m_screen(Screen::the())
    , m_connection(Connection::the())
    , m_compositor(Compositor::the())
    , m_event_loop(LFoundation::EventLoop::the())
{
    s_the = this;
}

void WindowManager::start_window_move(Window& window)
{
    m_movable_window = window.weak_ptr();
}

bool WindowManager::continue_window_move(MouseEvent* mouse_event)
{
    if (!movable_window()) {
        return false;
    }

    update_mouse_position(mouse_event);

    if (!is_mouse_left_button_pressed()) {
        m_movable_window = nullptr;
        return true;
    }

    auto bounds = movable_window()->bounds();
    m_compositor.invalidate(movable_window()->bounds());
    movable_window()->bounds().offset_by(mouse_event->packet().x_offset, -mouse_event->packet().y_offset);
    movable_window()->content_bounds().offset_by(mouse_event->packet().x_offset, -mouse_event->packet().y_offset);
    bounds.unite(movable_window()->bounds());
    m_compositor.invalidate(bounds);
    return true;
}

void WindowManager::update_mouse_position(MouseEvent* mouse_event)
{
    auto invalidate_bounds = m_compositor.cursor_manager().current_cursor().bounds();
    invalidate_bounds.origin().set(m_compositor.cursor_manager().draw_position(m_mouse_x, m_mouse_y));
    m_compositor.invalidate(invalidate_bounds);

    m_mouse_x += mouse_event->packet().x_offset;
    m_mouse_y -= mouse_event->packet().y_offset;
    if (m_mouse_x < 0) {
        m_mouse_x = 0;
    }
    if (m_mouse_y < 0) {
        m_mouse_y = 0;
    }
    if (m_mouse_x >= m_screen.width() - 1) {
        m_mouse_x = m_screen.width() - 1;
    }
    if (m_mouse_y >= m_screen.height() - 1) {
        m_mouse_y = m_screen.height() - 1;
    }

    invalidate_bounds.origin().set(m_compositor.cursor_manager().draw_position(m_mouse_x, m_mouse_y));
    m_compositor.invalidate(invalidate_bounds);

    m_mouse_changed_button_status = false;
    if (m_mouse_left_button_pressed != (mouse_event->packet().button_states & 1)) {
        m_mouse_changed_button_status = true;
    }

    m_mouse_left_button_pressed = (mouse_event->packet().button_states & 1);
}

void WindowManager::receive_mouse_event(UniquePtr<LFoundation::Event> event)
{
    MouseEvent* mouse_event = (MouseEvent*)event.release();
    auto new_hovered_window = WeakPtr<Window>();

    if (continue_window_move(mouse_event)) {
        goto end;
    }

    update_mouse_position(mouse_event);

    for (auto& window : m_windows) {
        if (window.bounds().contains(m_mouse_x, m_mouse_y)) {
            if (window.frame().bounds().contains(m_mouse_x, m_mouse_y)) {
                if (is_mouse_left_button_pressed()) {
                    auto tap_point = LG::Point<int>(m_mouse_x - window.frame().bounds().min_x(), m_mouse_y - window.frame().bounds().min_y());
                    window.frame().receive_tap_event(tap_point);
                    start_window_move(window);
                }
            } else if (window.content_bounds().contains(m_mouse_x, m_mouse_y)) {
                LG::Point<int> point(m_mouse_x, m_mouse_y);
                point.offset_by(-window.content_bounds().origin());
                m_event_loop.add(m_connection, new SendEvent(new MouseMoveMessage(window.connection_id(), window.id(), point.x(), point.y())));
                new_hovered_window = window.weak_ptr();

                if (m_mouse_changed_button_status) {
                    // FIXME: only left button for now!
                    m_event_loop.add(m_connection, new SendEvent(new MouseActionMessage(window.connection_id(), window.id(), !is_mouse_left_button_pressed(), point.x(), point.y())));
                }
            }

            if (is_mouse_left_button_pressed() && m_active_window.ptr() != &window) {
                bring_to_front(window);
                m_active_window = window.weak_ptr();
            }

            break;
        }
    }

    if (hovered_window()) {
        auto window = hovered_window();
        if (window->id() != new_hovered_window->id()) {
            m_event_loop.add(m_connection, new SendEvent(new MouseLeaveMessage(window->connection_id(), window->id(), 0, 0)));
        }
    }

    m_hovered_window = new_hovered_window;

end:
    delete mouse_event;
}

void WindowManager::receive_keyboard_event(UniquePtr<LFoundation::Event> event)
{
    KeyboardEvent* keyboard_event = (KeyboardEvent*)event.release();
    if (active_window()) {
        auto window = active_window();
        m_event_loop.add(m_connection, new SendEvent(new KeyboardMessage(window->connection_id(), window->id(), keyboard_event->packet().key)));
    }
    delete keyboard_event;
}

void WindowManager::receive_event(UniquePtr<LFoundation::Event> event)
{
    if (event->type() == WSEvent::Type::MouseEvent) {
        receive_mouse_event(move(event));
    }
    if (event->type() == WSEvent::Type::KeyboardEvent) {
        receive_keyboard_event(move(event));
    }
}