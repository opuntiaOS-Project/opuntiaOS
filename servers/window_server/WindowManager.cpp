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

    int is_pressed = (mouse_event->packet().button_states & 1);
    m_mouse_x += mouse_event->packet().x_offset;
    m_mouse_y -= mouse_event->packet().y_offset;

    if (!is_pressed) {
        m_movable_window = nullptr;
        return true;
    }
    
    auto bounds = m_movable_window->bounds();
    m_compositor.invalidate(m_movable_window->bounds());
    m_movable_window->bounds().offset_by(mouse_event->packet().x_offset, -mouse_event->packet().y_offset);
    bounds.unite(m_movable_window->bounds());
    m_compositor.invalidate(bounds);
    return true;
}

void WindowManager::receive_mouse_event(UniquePtr<LFoundation::Event> event)
{
    auto* mouse_event = (MouseEvent*)event.release();

    if (continue_window_move(mouse_event)) {
        delete mouse_event;
        return;
    }

    m_compositor.invalidate(LG::Rect(m_mouse_x, m_mouse_y, cursor_size(), cursor_size()));
    m_mouse_x += mouse_event->packet().x_offset;
    m_mouse_y -= mouse_event->packet().y_offset;
    if (m_mouse_x < 0) {
        m_mouse_x = 0;
    }
    if (m_mouse_y < 0) {
        m_mouse_y = 0;
    }
    if (m_mouse_x >= m_screen.width() - cursor_size()) {
        m_mouse_x = m_screen.width() - cursor_size();
    }
    if (m_mouse_y >= m_screen.height() - cursor_size()) {
        m_mouse_y = m_screen.height() - cursor_size();
    }
    m_compositor.invalidate(LG::Rect(m_mouse_x, m_mouse_y, cursor_size(), cursor_size()));

    m_is_pressed = (mouse_event->packet().button_states & 1);

    for (int i = 0; i < m_windows.size(); i++) {
        if (m_windows[i].frame().bounds().contains(m_mouse_x, m_mouse_y)) {
            if (m_is_pressed) {
                start_window_move(m_windows[i]);
                break;
            }
        } else if (m_windows[i].bounds().contains(m_mouse_x, m_mouse_y)) {
            m_event_loop.add(m_connection, new SendEvent(new MouseMessage(m_windows[i].connection_id(), m_windows[i].id(), m_mouse_x, m_mouse_y)));
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