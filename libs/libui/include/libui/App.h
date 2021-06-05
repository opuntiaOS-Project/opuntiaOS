/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libfoundation/Event.h>
#include <libfoundation/EventLoop.h>
#include <libfoundation/EventReceiver.h>
#include <libui/AppDelegate.h>
#include <libui/Connection.h>
#include <libui/Window.h>
#include <memory>
#include <sys/types.h>

namespace UI {

enum class AppState {
    Active,
    Background,
};

class App : public LFoundation::EventReceiver {
public:
    static App& the();
    App();

    inline int run() { return m_event_loop.run(); }
    inline LFoundation::EventLoop& event_loop() { return m_event_loop; }
    inline const LFoundation::EventLoop& event_loop() const { return m_event_loop; }
    void set_window(Window* window) { m_window = window; }
    inline Window& window() { return *m_window; }
    inline const Window& window() const { return *m_window; }

    void set_delegate(AppDelegate* delegate) { m_delegate = delegate; }
    inline AppDelegate* delegate() { return m_delegate; }

    inline const Connection& connection() const { return m_server_connection; }
    void receive_event(std::unique_ptr<LFoundation::Event> event) override;

    void set_state(AppState state) { m_state = state; }
    AppState state() const { return m_state; }

private:
    LFoundation::EventLoop m_event_loop;
    Connection m_server_connection;
    AppDelegate* m_delegate { nullptr };
    Window* m_window { nullptr };
    AppState m_state;
};

} // namespace UI