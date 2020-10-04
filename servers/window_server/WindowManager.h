/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "Screen.h"
#include "WSConnection.h"
#include "WSEvent.h"
#include "WSServerDecoder.h"
#include "Window.h"
#include <libfoundation/EventReceiver.h>
#include <libipc/ServerConnection.h>
#include <std/Vector.h>
#include <syscalls.h>

class WindowManager : public LFoundation::EventReceiver {
public:
    static WindowManager& the();
    WindowManager();

    inline void add_window(Window&& window) { m_windows.push_back(move(window)); }
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

    void receive_event(UniquePtr<LFoundation::Event> event) override
    {
        if (event->type() == WSEvent::Type::MouseEvent) {
            auto mouse_event = reinterpret_cast<MouseEvent*>(event.release());
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
            
            m_is_pressed = (mouse_event->packet().button_states & 1);
        }
    }

    inline int mouse_x() const { return m_mouse_x; }
    inline int mouse_y() const { return m_mouse_y; }
    inline bool is_cursor_pressed() const { return m_is_pressed; }
    constexpr int cursor_size() const { return 14; }

private:
    int m_mouse_x { 0 };
    int m_mouse_y { 0 };
    bool m_is_pressed { false };
    Vector<Window> m_windows;
    Screen& m_screen;
};