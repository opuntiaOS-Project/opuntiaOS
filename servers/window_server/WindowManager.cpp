/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "WindowManager.h"
#include "Screen.h"

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
    if (!m_movable_window) {
        return false;
    }

    update_mouse_position(mouse_event);

    if (!is_mouse_left_button_pressed()) {
        m_movable_window = nullptr;
        return true;
    }
    
    auto bounds = m_movable_window->bounds();
    m_compositor.invalidate(m_movable_window->bounds());
    m_movable_window->bounds().offset_by(mouse_event->packet().x_offset, -mouse_event->packet().y_offset);
    m_movable_window->content_bounds().offset_by(mouse_event->packet().x_offset, -mouse_event->packet().y_offset);
    bounds.unite(m_movable_window->bounds());
    m_compositor.invalidate(bounds);
    return true;
}

void WindowManager::update_mouse_position(MouseEvent* mouse_event)
{
    m_compositor.invalidate(LG::Rect(m_mouse_x, m_mouse_y, cursor_size(), cursor_size()));
    
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

    m_compositor.invalidate(LG::Rect(m_mouse_x, m_mouse_y, cursor_size(), cursor_size()));

    m_mouse_changed_button_status = false;
    if (m_mouse_left_button_pressed != (mouse_event->packet().button_states & 1)) {
        m_mouse_changed_button_status = true;
    }

    m_mouse_left_button_pressed = (mouse_event->packet().button_states & 1);
}

void WindowManager::receive_mouse_event(UniquePtr<LFoundation::Event> event)
{
    auto* mouse_event = (MouseEvent*)event.release();

    if (continue_window_move(mouse_event)) {
        delete mouse_event;
        return;
    }

    update_mouse_position(mouse_event);
    if (has_hovered_window()) {
        auto& window = hovered_window();
        if (!window.content_bounds().contains(m_mouse_x, m_mouse_y)) {
            LG::Point<int> point(m_mouse_x, m_mouse_y);
            point.offset_by(-m_windows[0].content_bounds().origin());
            m_event_loop.add(m_connection, new SendEvent(new MouseLeaveMessage(window.connection_id(), window.id(), point.x(), point.y())));
            m_hovered_window_id = -1;
        }
    }

    for (int i = m_windows.size() - 1; i >= 0; i--) {
        if (m_windows[i].frame().bounds().contains(m_mouse_x, m_mouse_y)) {
            if (is_mouse_left_button_pressed()) {
                start_window_move(m_windows[i]);
                break;
            }
        } else if (m_windows[i].content_bounds().contains(m_mouse_x, m_mouse_y)) {
            LG::Point<int> point(m_mouse_x, m_mouse_y);
            point.offset_by(-m_windows[i].content_bounds().origin());
            m_event_loop.add(m_connection, new SendEvent(new MouseMoveMessage(m_windows[i].connection_id(), m_windows[i].id(), point.x(), point.y())));
            m_hovered_window_id = m_windows[i].id();

            if (m_mouse_changed_button_status) {
                // FIXME: only left button for now!
                m_event_loop.add(m_connection, new SendEvent(new MouseActionMessage(m_windows[i].connection_id(), m_windows[i].id(), !is_mouse_left_button_pressed(), point.x(), point.y())));    
            }

            break;
        }
    }
    delete mouse_event;
}

void WindowManager::receive_event(UniquePtr<LFoundation::Event> event)
{
    if (event->type() == WSEvent::Type::MouseEvent) {
        receive_mouse_event(move(event));
    }
}