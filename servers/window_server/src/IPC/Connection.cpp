/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Connection.h"
#include "Event.h"
#include <libfoundation/EventLoop.h>
#include <sys/socket.h>

namespace WinServer {

Connection* s_WinServer_Connection_the = nullptr;

Connection::Connection(int connection_fd)
    : m_connection_fd(connection_fd)
    , m_server_decoder()
    , m_client_decoder()
    , m_connection_with_clients(m_connection_fd, m_server_decoder, m_client_decoder)
{
    s_WinServer_Connection_the = this;
    int err = bind(m_connection_fd, "/tmp/win.sock", 13);
    if (!err) {
        LFoundation::EventLoop::the().add(
            m_connection_fd, [] {
                Connection::the().listen();
            },
            nullptr);
    }
}

void Connection::receive_event(std::unique_ptr<LFoundation::Event> event)
{
    if (event->type() == WinServer::Event::Type::SendEvent) {
        std::unique_ptr<SendEvent> send_event = std::move(event);
        m_connection_with_clients.send_message(*send_event->message());
    }
}

} // namespace WinServer