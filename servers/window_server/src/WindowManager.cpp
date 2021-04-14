/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "WindowManager.h"
#include "CursorManager.h"
#include "Screen.h"
#include <libfoundation/KeyboardMapping.h>
#include <libfoundation/Logger.h>

// #define WM_DEBUG

namespace WinServer {

static WindowManager* s_the;

WindowManager& WindowManager::the()
{
    return *s_the;
}

WindowManager::WindowManager()
    : m_screen(Screen::the())
    , m_connection(Connection::the())
    , m_compositor(Compositor::the())
    , m_cursor_manager(CursorManager::the())
    , m_event_loop(LFoundation::EventLoop::the())
{
    s_the = this;
}

void WindowManager::start_window_move(Window& window)
{
    m_movable_window = window.weak_ptr();
}

void WindowManager::setup_dock(Window* window)
{
    window->make_frameless();
    window->bounds().set_y(m_screen.bounds().max_y() - window->bounds().height() + 1);
    window->content_bounds().set_y(m_screen.bounds().max_y() - window->bounds().height() + 1);
    window->set_event_mask(WindowEvent::IconChange | WindowEvent::WindowStatus);
    m_dock_window = window->weak_ptr();
}

bool WindowManager::continue_window_move()
{
    if (!movable_window()) {
        return false;
    }

    if (!m_cursor_manager.pressed<CursorManager::Params::LeftButton>()) {
        m_movable_window = nullptr;
        return true;
    }

    auto bounds = movable_window()->bounds();
    m_compositor.invalidate(movable_window()->bounds());
    movable_window()->offset_by(m_cursor_manager.get<CursorManager::Params::OffsetX>(), m_cursor_manager.get<CursorManager::Params::OffsetY>());
    bounds.unite(movable_window()->bounds());
    m_compositor.invalidate(bounds);
    return true;
}

void WindowManager::update_mouse_position(MouseEvent* mouse_event)
{
    auto invalidate_bounds = m_cursor_manager.current_cursor().bounds();
    invalidate_bounds.origin().set(m_cursor_manager.draw_position());
    m_compositor.invalidate(invalidate_bounds);

    m_cursor_manager.update_position(mouse_event);

    invalidate_bounds.origin().set(m_cursor_manager.draw_position());
    m_compositor.invalidate(invalidate_bounds);
}

void WindowManager::receive_mouse_event(std::unique_ptr<LFoundation::Event> event)
{
    auto new_hovered_window = WeakPtr<Window>();
    auto* mouse_event = (MouseEvent*)event.release();
    update_mouse_position(mouse_event);

    if (continue_window_move()) {
        goto end;
    }

    for (auto* window_ptr : m_windows) {
        auto& window = *window_ptr;
        if (window.bounds().contains(m_cursor_manager.x(), m_cursor_manager.y())) {
            if (window.frame().bounds().contains(m_cursor_manager.x(), m_cursor_manager.y())) {
                if (m_cursor_manager.pressed<CursorManager::Params::LeftButton>()) {
                    auto tap_point = LG::Point<int>(m_cursor_manager.x() - window.frame().bounds().min_x(), m_cursor_manager.y() - window.frame().bounds().min_y());
                    window.frame().receive_tap_event(tap_point);
                    start_window_move(window);
                }
            } else if (window.content_bounds().contains(m_cursor_manager.x(), m_cursor_manager.y())) {
                LG::Point<int> point(m_cursor_manager.x(), m_cursor_manager.y());
                point.offset_by(-window.content_bounds().origin());
                m_event_loop.add(m_connection, new SendEvent(new MouseMoveMessage(window.connection_id(), window.id(), point.x(), point.y())));
                new_hovered_window = window.weak_ptr();

                if (m_cursor_manager.is_changed<CursorManager::Params::Buttons>()) {
                    // FIXME: only left button for now!
                    bool is_left_pressed = m_cursor_manager.pressed<CursorManager::Params::LeftButton>();
                    m_event_loop.add(m_connection, new SendEvent(new MouseActionMessage(window.connection_id(), window.id(), !is_left_pressed, point.x(), point.y())));
                }
            }

            if (m_cursor_manager.pressed<CursorManager::Params::LeftButton>() && m_active_window.ptr() != &window) {
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

void WindowManager::receive_keyboard_event(std::unique_ptr<LFoundation::Event> event)
{
    auto* keyboard_event = (KeyboardEvent*)event.release();
    if (active_window()) {
        auto window = active_window();
        m_event_loop.add(m_connection, new SendEvent(new KeyboardMessage(window->connection_id(), window->id(), keyboard_event->packet().key)));
    }
    delete keyboard_event;
}

void WindowManager::receive_event(std::unique_ptr<LFoundation::Event> event)
{
    if (event->type() == WinServer::Event::Type::MouseEvent) {
        receive_mouse_event(std::move(event));
    } else if (event->type() == WinServer::Event::Type::KeyboardEvent) {
        receive_keyboard_event(std::move(event));
    }
}

// Notifiers

bool WindowManager::notify_listner_about_window_status(const Window& win, int changed_window_id, WindowStatusUpdateType type)
{
#ifdef WM_DEBUG
    Logger::debug << "notify_listner_about_window_status " << win.id() << " that " << changed_window_id << " " << type << std::endl;
#endif
    m_event_loop.add(m_connection, new SendEvent(new NotifyWindowStatusChangedMessage(win.connection_id(), win.id(), changed_window_id, (int)type)));
    return true;
}

bool WindowManager::notify_listner_about_changed_icon(const Window& win, int changed_window_id)
{
#ifdef WM_DEBUG
    Logger::debug << "notify_listner_about_changed_icon " << win.id() << " that " << changed_window_id << std::endl;
#endif
    auto* changed_window_ptr = window(changed_window_id);
    if (!changed_window_ptr) {
        return false;
    }
    m_event_loop.add(m_connection, new SendEvent(new NotifyWindowIconChangedMessage(win.connection_id(), win.id(), changed_window_id, changed_window_ptr->icon_path())));
    return true;
}

void WindowManager::notify_window_status_changed(int changed_window_id, WindowStatusUpdateType type)
{
    for (auto* window_ptr : m_windows) {
        auto& window = *window_ptr;
        if (window.event_mask() & WindowEvent::WindowStatus) {
            notify_listner_about_window_status(window, changed_window_id, type);
        }
    }
}

void WindowManager::notify_window_icon_changed(int changed_window_id)
{
    for (auto* window_ptr : m_windows) {
        auto& window = *window_ptr;
        if (window.event_mask() & WindowEvent::IconChange) {
            notify_listner_about_changed_icon(window, changed_window_id);
        }
    }
}

} // namespace WinServer