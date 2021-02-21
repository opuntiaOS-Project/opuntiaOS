/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Connection.h"
#include "WSEvent.h"
#include <libfoundation/EventLoop.h>

static Connection* s_the;

Connection& Connection::the()
{
    return *s_the;
}

Connection::Connection(int connection_fd)
    : m_connection_fd(connection_fd)
    , m_server_decoder()
    , m_client_decoder()
    , m_connection_with_clients(m_connection_fd, m_server_decoder, m_client_decoder)
{
    s_the = this;
    int err = bind(m_connection_fd, "/win.sock", 9);
    if (!err) {
        LFoundation::EventLoop::the().add(
            m_connection_fd, [] {
                    Connection::the().listen();
                },
            nullptr);
    }
}

void Connection::receive_event(UniquePtr<LFoundation::Event> event)
{
    if (event->type() == WSEvent::Type::SendEvent) {
        UniquePtr<SendEvent> send_event = move(event);
        m_connection_with_clients.send_message(*send_event->message());
    }
}