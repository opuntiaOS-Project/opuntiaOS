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
#include <std/Vector.h>
#include <syscalls.h>

class WindowManager : public LFoundation::EventReceiver {
public:
    static WindowManager& the();
    WindowManager();

    inline void add_window(Window&& window)
    {
        m_compositor.invalidate(window.bounds());
        m_windows.push_back(move(window));
    }
    
    inline Window& window(int id)
    {
        for (int i = 0; i < windows().size(); i++) {
            if (windows()[i].id() == id) {
                return windows()[i];
            }
        }
        return windows()[0];
    }

    inline Vector<Window>& windows() { return m_windows; }
    inline const Vector<Window>& windows() const { return m_windows; }

    void receive_event(UniquePtr<LFoundation::Event> event) override;

    inline int mouse_x() const { return m_mouse_x; }
    inline int mouse_y() const { return m_mouse_y; }
    inline bool is_cursor_pressed() const { return m_is_pressed; }
    constexpr int cursor_size() const { return 14; }

private:
    void start_window_move(Window& window);
    bool continue_window_move(MouseEvent* mouse_event);

    void receive_mouse_event(UniquePtr<LFoundation::Event> event);

    int m_mouse_x { 0 };
    int m_mouse_y { 0 };
    bool m_is_pressed { false };
    Vector<Window> m_windows;
    Screen& m_screen;
    Connection& m_connection;
    Compositor& m_compositor;
    LFoundation::EventLoop& m_event_loop;

    WeakPtr<Window> m_movable_window;
};