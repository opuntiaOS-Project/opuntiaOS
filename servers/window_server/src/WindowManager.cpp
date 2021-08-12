/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "WindowManager.h"
#include "../shared/MessageContent/MouseAction.h"
#include "CursorManager.h"
#include "Screen.h"
#include <libfoundation/KeyboardMapping.h>
#include <libfoundation/Logger.h>

// #define WM_DEBUG

namespace WinServer {

static PopupData WindowPopupData {};

WindowManager* s_WinServer_WindowManager_the = nullptr;

WindowManager::WindowManager()
    : m_screen(Screen::the())
    , m_connection(Connection::the())
    , m_compositor(Compositor::the())
    , m_cursor_manager(CursorManager::the())
    , m_event_loop(LFoundation::EventLoop::the())
    , m_std_menubar_content()
{
    s_WinServer_WindowManager_the = this;
    m_std_menubar_content.push_back(MenuDir("opuntiaOS", 0));
}

void WindowManager::start_window_move(Window& window)
{
    m_movable_window = &window;
}

void WindowManager::setup_dock(Window* window)
{
#ifdef TARGET_DESKTOP
    window->make_frameless();
    window->bounds().set_y(m_screen.bounds().max_y() - window->bounds().height() + 1);
    window->content_bounds().set_y(m_screen.bounds().max_y() - window->bounds().height() + 1);
    window->set_event_mask(WindowEvent::IconChange | WindowEvent::WindowStatus);
#endif // TARGET_DESKTOP
    m_dock_window = window;
}

void WindowManager::add_window(Window* window)
{
    if (window->type() == WindowType::Homescreen) {
        setup_dock(window);
    }
    m_windows.push_back(window);
    set_active_window(window);
    notify_window_status_changed(window->id(), WindowStatusUpdateType::Created);
}

void WindowManager::remove_window_from_screen(Window* window)
{
    if (movable_window() == window) {
        m_movable_window = nullptr;
    }
    if (active_window() == window) {
        m_compositor.menu_bar().set_menubar_content(&m_std_menubar_content, m_compositor);
        m_active_window = nullptr;
    }
    if (hovered_window() == window) {
        m_hovered_window = nullptr;
    }
    m_compositor.invalidate(window->bounds());
}

void WindowManager::remove_window(Window* window)
{
    if (m_dock_window == window) {
        return;
    }
    remove_window_from_screen(window);
    m_windows.erase(std::find(m_windows.begin(), m_windows.end(), window));
    notify_window_status_changed(window->id(), WindowStatusUpdateType::Removed);
#ifdef TARGET_MOBILE
    if (auto* top_window = get_top_standard_window_in_view(); top_window) {
        m_active_window = top_window;
    }
#endif
    delete window;
}
void WindowManager::minimize_window(Window& window)
{
    Window* window_ptr = &window;
    remove_window_from_screen(window_ptr);
    window.set_visible(false);
    m_windows.erase(std::find(m_windows.begin(), m_windows.end(), window_ptr));
    m_windows.push_back(window_ptr);
}

WindowManager::Window* WindowManager::get_top_standard_window_in_view() const
{
    if (m_windows.empty()) {
        return nullptr;
    }

    auto it = m_windows.begin();
    if (m_dock_window) {
        it++;
    }

    if (it == m_windows.end()) {
        return *m_windows.begin();
    }
    return *it;
}

void WindowManager::bring_to_front(Window& window)
{
    auto* prev_window = get_top_standard_window_in_view();
    do_bring_to_front(window);
#ifdef TARGET_DESKTOP
    if (m_dock_window) {
        do_bring_to_front(*m_dock_window);
    }
    window.set_visible(true);
    window.frame().set_active(true);
    m_compositor.invalidate(window.bounds());
    if (prev_window && prev_window->id() != window.id()) {
        prev_window->frame().set_active(false);
        prev_window->frame().invalidate(m_compositor);
    }
    if (window.type() == WindowType::Standard) {
        m_compositor.menu_bar().set_menubar_content(&window.menubar_content(), m_compositor);
    }
#elif TARGET_MOBILE
    m_active_window = &window;
#endif // TARGET_DESKTOP
}

#ifdef TARGET_DESKTOP
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
    move_window(movable_window(), m_cursor_manager.get<CursorManager::Params::OffsetX>(), m_cursor_manager.get<CursorManager::Params::OffsetY>());
    bounds.unite(movable_window()->bounds());
    m_compositor.invalidate(bounds);
    return true;
}
#endif // TARGET_DESKTOP

void WindowManager::update_mouse_position(std::unique_ptr<LFoundation::Event> mouse_event)
{
    auto invalidate_bounds = m_cursor_manager.current_cursor().bounds();
    invalidate_bounds.origin().set(m_cursor_manager.draw_position());
    m_compositor.invalidate(invalidate_bounds);

    m_cursor_manager.update_position((WinServer::MouseEvent*)mouse_event.get());

    invalidate_bounds.origin().set(m_cursor_manager.draw_position());
    m_compositor.invalidate(invalidate_bounds);
}

#ifdef TARGET_DESKTOP
void WindowManager::receive_mouse_event(std::unique_ptr<LFoundation::Event> event)
{
    Window* new_hovered_window = nullptr;
    update_mouse_position(std::move(event));

    if (continue_window_move()) {
        return;
    }

    if (m_compositor.popup().visible() && m_compositor.popup().bounds().contains(m_cursor_manager.x(), m_cursor_manager.y())) {
        m_compositor.popup().on_mouse_move(m_cursor_manager);
        if (m_cursor_manager.is_changed<CursorManager::Params::Buttons>()) {
            m_compositor.popup().on_mouse_status_change(m_cursor_manager);
        }
        return;
    } else {
        m_compositor.popup().on_mouse_leave(m_cursor_manager);
        if (m_cursor_manager.pressed<CursorManager::Params::LeftButton>()) {
            m_compositor.popup().set_visible(false);
        }
    }

    // Checking and dispatching mouse move for MenuBar.
    if (m_compositor.menu_bar().bounds().contains(m_cursor_manager.x(), m_cursor_manager.y())) {
        m_compositor.menu_bar().on_mouse_move(m_cursor_manager);
        if (m_cursor_manager.is_changed<CursorManager::Params::Buttons>()) {
            m_compositor.menu_bar().on_mouse_status_change(m_cursor_manager);
        }
    } else if (m_compositor.menu_bar().is_hovered()) {
        m_compositor.menu_bar().on_mouse_leave(m_cursor_manager);
    }

    // Checking and dispatching mouse move for windows/
    for (auto* window_ptr : m_windows) {
        auto& window = *window_ptr;
        if (!window.visible()) {
            continue;
        }

        if (window.bounds().contains(m_cursor_manager.x(), m_cursor_manager.y())) {
            if (m_cursor_manager.pressed<CursorManager::Params::LeftButton>() && m_active_window != &window) {
                set_active_window(window);
            }
            if (m_cursor_manager.pressed<CursorManager::Params::RightButton>() && m_active_window != &window) {
                set_active_window(window);
            }

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
                new_hovered_window = &window;

                if (m_cursor_manager.is_changed<CursorManager::Params::Buttons>()) {
                    auto buttons_state = MouseActionState();
                    if (m_cursor_manager.is_changed<CursorManager::Params::LeftButton>()) {
                        // TODO: May be remove if?
                        if (m_cursor_manager.pressed<CursorManager::Params::LeftButton>()) {
                            buttons_state.set(MouseActionType::LeftMouseButtonPressed);
                        } else {
                            buttons_state.set(MouseActionType::LeftMouseButtonReleased);
                        }
                    }

                    m_event_loop.add(m_connection, new SendEvent(new MouseActionMessage(window.connection_id(), window.id(), buttons_state.state(), point.x(), point.y())));
                }
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
}
#elif TARGET_MOBILE
void WindowManager::receive_mouse_event(std::unique_ptr<LFoundation::Event> event)
{
    update_mouse_position(std::move(event));

    if (m_compositor.control_bar().control_button_bounds().contains(m_cursor_manager.x(), m_cursor_manager.y()) && active_window()) {
        if (m_cursor_manager.pressed<CursorManager::Params::LeftButton>()) {
            remove_window(active_window());
        }
        return;
    }

    // Tap emulation
    if (m_cursor_manager.is_changed<CursorManager::Params::Buttons>() && active_window()) {
        auto window = active_window();
        auto buttons_state = MouseActionState();
        LG::Point<int> point(m_cursor_manager.x(), m_cursor_manager.y());
        point.offset_by(-window->content_bounds().origin());
        if (m_cursor_manager.is_changed<CursorManager::Params::LeftButton>()) {
            // TODO: May be remove if?
            if (m_cursor_manager.pressed<CursorManager::Params::LeftButton>()) {
                buttons_state.set(MouseActionType::LeftMouseButtonPressed);
            } else {
                buttons_state.set(MouseActionType::LeftMouseButtonReleased);
            }
        }
        m_event_loop.add(m_connection, new SendEvent(new MouseActionMessage(window->connection_id(), window->id(), buttons_state.state(), point.x(), point.y())));
    }
}
#endif // TARGET_MOBILE

void WindowManager::receive_keyboard_event(std::unique_ptr<LFoundation::Event> event)
{
    auto* keyboard_event = reinterpret_cast<KeyboardEvent*>(event.release());
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
#ifdef TARGET_DESKTOP
#ifdef WM_DEBUG
    Logger::debug << "notify_listner_about_changed_icon " << win.id() << " that " << changed_window_id << std::endl;
#endif
    auto* changed_window_ptr = window(changed_window_id);
    if (!changed_window_ptr) {
        return false;
    }
    m_event_loop.add(m_connection, new SendEvent(new NotifyWindowIconChangedMessage(win.connection_id(), win.id(), changed_window_id, changed_window_ptr->icon_path())));
#endif
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