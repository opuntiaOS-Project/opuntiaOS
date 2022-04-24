/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Connection.h"
#include "Event.h"
#include <libfoundation/EventLoop.h>
#include <sys/socket.h>

#define WINSERVER_REQUEST_SOCKET_PATH "/tmp/winserver_requests.sock"
#define WINSERVER_REQUEST_SOCKET_PATH_SIZE sizeof(WINSERVER_REQUEST_SOCKET_PATH)

#define WINSERVER_RESPONSE_SOCKET_PATH "/tmp/winserver_response.sock"
#define WINSERVER_RESPONSE_SOCKET_PATH_SIZE sizeof(WINSERVER_RESPONSE_SOCKET_PATH)

namespace WinServer {

Connection* s_WinServer_Connection_the = nullptr;

Connection::Connection(const LIPC::DoubleSidedConnection& conn)
    : m_connection(conn)
    , m_server_decoder()
    , m_client_decoder()
    , m_connection_with_clients(m_connection, m_server_decoder, m_client_decoder)
{
    s_WinServer_Connection_the = this;
    int err1 = bind(m_connection.c2s_fd(), WINSERVER_REQUEST_SOCKET_PATH, WINSERVER_REQUEST_SOCKET_PATH_SIZE);
    int err2 = bind(m_connection.s2c_fd(), WINSERVER_RESPONSE_SOCKET_PATH, WINSERVER_RESPONSE_SOCKET_PATH_SIZE);

    if (!err1 && !err2) {
        LFoundation::EventLoop::the().add(
            m_connection.c2s_fd(), [] {
                Connection::the().listen();
            },
            nullptr);
    }
}

void Connection::receive_event(std::unique_ptr<LFoundation::Event> event)
{
    switch (event->type()) {
    case WinServer::Event::Type::SendEvent: {
        std::unique_ptr<SendEvent> send_event = std::move(event);
        m_connection_with_clients.send_message(*send_event->message());
        break;
    }
    }
}

} // namespace WinServer