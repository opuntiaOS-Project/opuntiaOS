/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "Connection.h"
#include "Window.h"
#include <libfoundation/Event.h>
#include <libfoundation/EventLoop.h>
#include <libfoundation/EventReceiver.h>
#include <memory>
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
    Window* m_window { nullptr };
};

}