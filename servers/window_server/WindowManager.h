/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "Compositor.h"
#include "Connection.h"
#include "Screen.h"
#include "WSConnection.h"
#include "WSEvent.h"
#include "WSServerDecoder.h"
#include "Window.h"
#include <libfoundation/EventLoop.h>
#include <libfoundation/EventReceiver.h>
#include <libipc/ServerConnection.h>
#include <std/LinkedList.h>
#include <std/Vector.h>
#include <syscalls.h>

class WindowManager : public LFoundation::EventReceiver {
public:
    static WindowManager& the();
    WindowManager();

    inline void add_window(Window* window)
    {
        m_compositor.invalidate(window->bounds());
        m_windows.push_front(window);
    }

    void close_window(Window& window)
    {
        // window->will_be_closed();
        // for (int i = 0; i < windows().size(); i++) {
        // }
    }

    inline Window* window(int id)
    {
        for (auto& window : m_windows) {
            if (window.id() == id) {
                return &window;
            }
        }
        return nullptr;
    }

    inline void bring_to_front(Window& window)
    {
        auto* window_ptr = &window;
        m_windows.remove(window_ptr);
        m_windows.push_front(window_ptr);
        m_compositor.invalidate(window.bounds());
    }

    inline LinkedList<Window>& windows() { return m_windows; }
    inline const LinkedList<Window>& windows() const { return m_windows; }
    inline int next_win_id() { return ++m_next_win_id; }

    void receive_event(UniquePtr<LFoundation::Event> event) override;

    inline int mouse_x() const { return m_mouse_x; }
    inline int mouse_y() const { return m_mouse_y; }
    inline bool is_mouse_left_button_pressed() const { return m_mouse_left_button_pressed; }
    constexpr int cursor_size() const { return 14; }

private:
    void start_window_move(Window& window);
    bool continue_window_move(MouseEvent* mouse_event);

    void update_mouse_position(MouseEvent* mouse_event);
    void receive_mouse_event(UniquePtr<LFoundation::Event> event);
    void receive_keyboard_event(UniquePtr<LFoundation::Event> event);

    inline bool has_hovered_window() const { return m_hovered_window_id != -1; }
    inline void set_hovered_window(int hw) { m_hovered_window_id = hw; }
    inline Window& hovered_window() { return *window(m_hovered_window_id); }
    inline bool has_active_window() const { return m_active_window_id != -1; }
    inline Window& active_window() { return *window(m_active_window_id); }

    LinkedList<Window> m_windows;

    Screen& m_screen;
    Connection& m_connection;
    Compositor& m_compositor;
    LFoundation::EventLoop& m_event_loop;

    WeakPtr<Window> m_movable_window;
    int m_active_window_id { -1 };
    int m_hovered_window_id { -1 };
    int m_next_win_id { 0 };

    int m_mouse_x { 0 };
    int m_mouse_y { 0 };
    bool m_mouse_left_button_pressed { false };
    bool m_mouse_changed_button_status { false };
};