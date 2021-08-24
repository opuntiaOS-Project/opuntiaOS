/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/Logger.h>
#include <libipc/ClientConnection.h>
#include <libui/Connection.h>
#include <libui/Window.h>
#include <memory>
#include <new>
#include <sched.h>
#include <sys/socket.h>

// #define DEBUG_CONNECTION

namespace UI {

static Connection* s_the = nullptr;

Connection& Connection::the()
{
    // FIXME: Thread-safe method to be applied
    if (!s_the) {
        new Connection(socket(PF_LOCAL, 0, 0));
    }
    return *s_the;
}

Connection::Connection(int connection_fd)
    : m_connection_fd(connection_fd)
    , m_server_decoder()
    , m_client_decoder()
    , m_connection_with_server(m_connection_fd, m_server_decoder, m_client_decoder)
{
    s_the = this;
    if (m_connection_fd > 0) {
        bool connected = false;
        // Trying to connect for 5 times. If unsuccesfull, it crashes.
        for (int i = 0; i < 5; i++) {
            if (connect(m_connection_fd, "/tmp/win.sock", 9) == 0) {
                connected = true;
                break;
            }
            sched_yield();
        }
        if (!connected) {
            goto crash;
        }

        greeting();
        setup_listners();
        return;
    }
crash:
    exit(-1);
}

void Connection::setup_listners()
{
    LFoundation::EventLoop::the().add(
        m_connection_fd, [] {
            Connection::the().listen();
        },
        nullptr);
}

void Connection::greeting()
{
    auto resp_message = send_sync_message<GreetMessageReply>(GreetMessage(getpid()));
    m_connection_id = resp_message->connection_id();
    m_connection_with_server.set_accepted_key(m_connection_id);
#ifdef DEBUG_CONNECTION
    Logger::debug << "Got greet with server" << std::endl;
#endif
}

int Connection::new_window(const Window& window)
{
    auto message = CreateWindowMessage(key(), window.type(), window.bounds().width(), window.bounds().height(), window.buffer().id(), window.icon_path());
    auto resp_message = send_sync_message<CreateWindowMessageReply>(message);
#ifdef DEBUG_CONNECTION
    Logger::debug << "New window created" << std::endl;
#endif
    return resp_message->window_id();
}
} // namespace UI