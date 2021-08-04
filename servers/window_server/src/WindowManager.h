/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../shared/Connections/WSConnection.h"
#include "Components/ControlBar/ControlBar.h"
#include "Components/MenuBar/MenuBar.h"
#include "Compositor.h"
#include "Connection.h"
#ifdef TARGET_DESKTOP
#include "Desktop/Window.h"
#elif TARGET_MOBILE
#include "Mobile/Window.h"
#endif
#include "Event.h"
#include "Screen.h"
#include "ServerDecoder.h"
#include <algorithm>
#include <libfoundation/EventLoop.h>
#include <libfoundation/EventReceiver.h>
#include <libipc/ServerConnection.h>
#include <list>
#include <vector>

namespace WinServer {

class WindowManager : public LFoundation::EventReceiver {
#ifdef TARGET_DESKTOP
    using Window = WinServer::Desktop::Window;
#elif TARGET_MOBILE
    using Window = WinServer::Mobile::Window;
#endif

public:
    inline static WindowManager& the()
    {
        extern WindowManager* s_WinServer_WindowManager_the;
        return *s_WinServer_WindowManager_the;
    }

    WindowManager();

    inline void add_window(Window* window)
    {
        if (window->type() == WindowType::Homescreen) {
            setup_dock(window);
        }
        m_windows.push_back(window);
        bring_to_front(*window);
        notify_window_status_changed(window->id(), WindowStatusUpdateType::Created);
    }

    void remove_window(Window* window);

    void close_window(Window& window) { m_event_loop.add(m_connection, new SendEvent(new WindowCloseRequestMessage(window.connection_id(), window.id()))); }
    void minimize_window(Window& window)
    {
        Window* window_ptr = &window;
        remove_window_from_screen(window_ptr);
        window.set_visible(false);
        m_windows.erase(std::find(m_windows.begin(), m_windows.end(), window_ptr));
        m_windows.push_back(window_ptr);
    }

    inline Window* window(int id)
    {
        for (auto* window : m_windows) {
            if (window->id() == id) {
                return window;
            }
        }
        return nullptr;
    }

    inline void do_bring_to_front(Window& window)
    {
        auto* window_ptr = &window;
        m_windows.erase(std::find(m_windows.begin(), m_windows.end(), window_ptr));
        m_windows.push_front(window_ptr);
    }

    Window* get_top_standard_window_in_view() const
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

    void bring_to_front(Window& window)
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

    inline std::list<Window*>& windows() { return m_windows; }
    inline const std::list<Window*>& windows() const { return m_windows; }
    inline int next_win_id() { return ++m_next_win_id; }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;

    void setup_dock(Window* window);

    // Notifiers
    bool notify_listner_about_window_status(const Window& window, int changed_window_id, WindowStatusUpdateType type);
    bool notify_listner_about_changed_icon(const Window&, int changed_window_id);

    void notify_window_status_changed(int changed_window_id, WindowStatusUpdateType type);
    void notify_window_icon_changed(int changed_window_id);

    void move_window(Window* window, int x_offset, int y_offset)
    {
        y_offset = std::max(y_offset, (int)m_compositor.menu_bar().height() - (int)Desktop::WindowFrame::std_top_border_frame_size() - window->bounds().min_y());
        if (m_dock_window) [[likely]] {
            y_offset = std::min(y_offset, (int)(m_screen.height() - window->content_bounds().min_y() - m_dock_window->bounds().height()));
        }
        window->bounds().offset_by(x_offset, y_offset);
        window->content_bounds().offset_by(x_offset, y_offset);
    }

private:
    void remove_window_from_screen(Window* window);

    void start_window_move(Window& window);
    bool continue_window_move();

    void update_mouse_position(std::unique_ptr<LFoundation::Event> mouse_event);
    void receive_mouse_event(std::unique_ptr<LFoundation::Event> event);
    void receive_keyboard_event(std::unique_ptr<LFoundation::Event> event);

    inline Window* movable_window() { return m_movable_window; }
    inline Window* hovered_window() { return m_hovered_window; }
    inline Window* active_window() { return m_active_window; }

    std::list<Window*> m_windows;

    Screen& m_screen;
    Connection& m_connection;
    Compositor& m_compositor;
    CursorManager& m_cursor_manager;
    LFoundation::EventLoop& m_event_loop;
    std::vector<MenuDir> m_std_menubar_content;

    // TODO: implement with std::weak_ptr.
    Window* m_dock_window {};
    Window* m_movable_window {};
    Window* m_active_window {};
    Window* m_hovered_window {};
    int m_next_win_id { 0 };
};

} // namespace WinServer