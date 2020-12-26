/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "Connection.h"
#include "Window.h"
#include <libfoundation/Event.h>
#include <libfoundation/EventLoop.h>
#include <libfoundation/EventReceiver.h>
#include <sys/types.h>

namespace UI {

class App : public LFoundation::EventReceiver {
public:
    static App& the();
    App();

    inline int run() { return m_event_loop.run(); }
    void set_window(Window* window) { m_window = window; }
    inline Window& window() { return *m_window; }
    inline const Window& window() const { return *m_window; }

    inline const Connection& connection() const { return m_server_connection; }
    void receive_event(UniquePtr<LFoundation::Event> event) override;

private:
    LFoundation::EventLoop m_event_loop;
    Connection m_server_connection;
    Window* m_window;
};

}