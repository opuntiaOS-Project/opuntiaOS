/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/Logger.h>
#include <libfoundation/ProcessInfo.h>
#include <libipc/ClientConnection.h>
#include <libui/Connection.h>
#include <libui/Window.h>
#include <memory>
#include <new>
#include <sched.h>
#include <sys/socket.h>

// #define DEBUG_CONNECTION

namespace UI {

#define WINSERVER_REQUEST_SOCKET_PATH "/tmp/winserver_requests.sock"
#define WINSERVER_REQUEST_SOCKET_PATH_SIZE sizeof(WINSERVER_REQUEST_SOCKET_PATH)

#define WINSERVER_RESPONSE_SOCKET_PATH "/tmp/winserver_response.sock"
#define WINSERVER_RESPONSE_SOCKET_PATH_SIZE sizeof(WINSERVER_RESPONSE_SOCKET_PATH)

static Connection* s_the = nullptr;

Connection& Connection::the()
{
    // FIXME: Thread-safe method to be applied
    if (!s_the) {
        auto conn = LIPC::DoubleSidedConnection(socket(PF_LOCAL, 0, 0), socket(PF_LOCAL, 0, 0));
        new Connection(conn);
    }
    return *s_the;
}

Connection::Connection(const LIPC::DoubleSidedConnection& connection)
    : m_connection(connection)
    , m_server_decoder()
    , m_client_decoder()
    , m_connection_with_server(m_connection, m_server_decoder, m_client_decoder)
{
    s_the = this;
    if (m_connection.c2s_fd() > 0 && m_connection.s2c_fd() > 0) {
        bool req_connected = false;
        bool resp_connected = false;
        // Trying to connect for 100 times. If unsuccesfull, it crashes.
        for (int i = 0; i < 100; i++) {
            if (!req_connected) {
                if (connect(m_connection.c2s_fd(), WINSERVER_REQUEST_SOCKET_PATH, WINSERVER_REQUEST_SOCKET_PATH_SIZE) == 0) {
                    req_connected = true;
                }
            }
            if (!resp_connected) {
                if (connect(m_connection.s2c_fd(), WINSERVER_RESPONSE_SOCKET_PATH, WINSERVER_RESPONSE_SOCKET_PATH_SIZE) == 0) {
                    resp_connected = true;
                }
            }

            if (req_connected && resp_connected) {
                break;
            }
            sched_yield();
        }

        if (!req_connected || !resp_connected) {
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
        m_connection.s2c_fd(), [] {
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
    const std::string& bundle_id = LFoundation::ProcessInfo::the().bundle_id();
    auto message = CreateWindowMessage(key(), window.type(), window.native_bounds().width(), window.native_bounds().height(),
        window.buffer().id(), window.title(), window.icon_path(), bundle_id,
        window.status_bar_style().color().u32(), window.status_bar_style().flags());
    auto resp_message = send_sync_message<CreateWindowMessageReply>(message);
#ifdef DEBUG_CONNECTION
    Logger::debug << "New window created" << std::endl;
#endif
    return resp_message->window_id();
}
} // namespace UI