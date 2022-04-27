/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../Components/ControlBar/ControlBar.h"
#include "../Components/MenuBar/MenuBar.h"
#include "../Components/Security/Violations.h"
#include "../Devices/Screen.h"
#include "../IPC/Connection.h"
#include "../IPC/Event.h"
#include "../IPC/ServerDecoder.h"
#include "../Managers/Compositor.h"
#include "../SystemApps/SystemApp.h"
#include "../Target/Generic/Window.h"
#include <algorithm>
#include <libapi/window_server/Connections/WSConnection.h>
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

    void add_window(Window* window);
    void remove_window(Window* window);

    void resize_window(Window& window, const LG::Size& size);
    void close_window(Window& window) { send_event(new WindowCloseRequestMessage(window.connection_id(), window.id())); }
    void minimize_window(Window& window);
    void maximize_window(Window& window);

    template <typename Callback>
    void minimize_windows(Callback callback)
    {
        std::vector<Window*> hided;
        for (auto* window : m_windows) {
            if (window && window->type() == WindowType::Standard && callback(window)) {
                window->set_visible(false);
                hided.push_back(window);
                on_window_became_invisible(window);
                remove_attention_from_window(window);
            }
        }

        for (auto* window : hided) {
            m_windows.erase(std::find(m_windows.begin(), m_windows.end(), window));
            m_windows.push_back(window);
        }
    }

    Window* top_window_in_view(WindowType type) const;
    inline Window* window(int id)
    {
        for (auto* window : m_windows) {
            if (window->id() == id) {
                return window;
            }
        }
        return nullptr;
    }

    inline void move_window(Window* window, int x_offset, int y_offset)
    {
        y_offset = std::max(y_offset, (int)visible_area().min_y() - (int)Desktop::WindowFrame::std_top_border_frame_size() - window->bounds().min_y());
        if (m_dock.has_value()) [[likely]] {
            y_offset = std::min(y_offset, (int)(visible_area().max_y() - window->content_bounds().min_y()));
        }
        window->bounds().offset_by(x_offset, y_offset);
        window->content_bounds().offset_by(x_offset, y_offset);
    }

    inline void do_bring_to_front(Window& window)
    {
        auto* window_ptr = &window;
        m_windows.erase(std::find(m_windows.begin(), m_windows.end(), window_ptr));
        m_windows.push_front(window_ptr);
    }
    void bring_to_front(Window& window);

    inline std::list<Window*>& windows() { return m_windows; }
    inline const std::list<Window*>& windows() const { return m_windows; }
    inline int next_win_id() { return ++m_next_win_id; }

    const LG::Rect& visible_area() const { return m_visible_area; }
    void shrink_visible_area(int top, int bottom) { m_visible_area.set_y(m_visible_area.min_y() + top), m_visible_area.set_height(m_visible_area.height() - top - bottom); }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override;

    // Notifiers
    bool notify_listner_about_window_creation(const Window& window, int changed_window_id);
    bool notify_listner_about_window_status(const Window& window, int changed_window_id, WindowStatusUpdateType type);
    bool notify_listner_about_changed_icon(const Window&, int changed_window_id);
    bool notify_listner_about_changed_title(const Window&, int changed_window_id);

    void notify_window_creation(int changed_window_id);
    void notify_window_status_changed(int changed_window_id, WindowStatusUpdateType type);
    void notify_window_icon_changed(int changed_window_id);
    void notify_window_title_changed(int changed_window_id);

    inline void ask_to_set_active_window(Window* win) { set_active_window(win); }
    inline void ask_to_set_active_window(Window& win) { set_active_window(win); }

    void on_window_misbehave(Window& window, ViolationClass);
    void on_window_style_change(Window& win);
    void on_window_menubar_change(Window& window);

    // Popup & Menubar
    inline Popup& popup() { return m_compositor.popup(); }
    inline const Popup& popup() const { return m_compositor.popup(); }
    inline MenuBar& menu_bar() { return m_compositor.menu_bar(); }
    inline const MenuBar& menu_bar() const { return m_compositor.menu_bar(); }

private:
    void add_system_window(Window* window);
    void bring_system_windows_to_front();
    void setup_dock(Window* window);
    void setup_applist(Window* window);

    void remove_attention_from_window(Window* window);

    void start_window_move(Window& window);
    bool continue_window_move();

    void update_mouse_position(std::unique_ptr<LFoundation::Event> mouse_event);
    void receive_mouse_event(std::unique_ptr<LFoundation::Event> event);
    void receive_keyboard_event(std::unique_ptr<LFoundation::Event> event);

    inline Window* movable_window() { return m_movable_window; }
    inline Window* hovered_window() { return m_hovered_window; }
    inline void set_hovered_window(Window* win) { m_hovered_window = win; }

    inline Window* active_window() { return m_active_window; }
    inline void set_active_window(Window* win) { on_active_window_will_change(), bring_to_front(*win), m_active_window = win, on_active_window_did_change(); }
    inline void set_active_window(Window& win) { on_active_window_will_change(), bring_to_front(win), m_active_window = &win, on_active_window_did_change(); }
    inline void set_active_window(std::nullptr_t) { on_active_window_will_change(), m_active_window = nullptr, on_active_window_did_change(); }

    void on_window_became_invisible(Window* window);
    void on_active_window_will_change();
    void on_active_window_did_change();

    inline void send_event(Message* msg) { m_event_loop.add(m_connection, new SendEvent(msg)); }

    std::list<Window*> m_windows;

    Screen& m_screen;
    Connection& m_connection;
    Compositor& m_compositor;
    CursorManager& m_cursor_manager;
    LFoundation::EventLoop& m_event_loop;
    std::vector<MenuDir> m_std_menubar_content;

    LG::Rect m_visible_area;

    SystemApp m_dock;
    SystemApp m_applist;

    // TODO: implement with std::weak_ptr.
    Window* m_movable_window {};
    Window* m_active_window {};
    Window* m_hovered_window {};
    int m_next_win_id { 0 };
};

} // namespace WinServer