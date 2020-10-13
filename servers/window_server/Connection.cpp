/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
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
    bind(m_connection_fd, "/win.sock", 9);
    LFoundation::EventLoop::the().add(
        m_connection_fd, [] {
            Connection::the().listen();
        },
        nullptr);
}

void Connection::receive_event(UniquePtr<LFoundation::Event> event)
{
    if (event->type() == WSEvent::Type::SendEvent) {
        UniquePtr<SendEvent> send_event = move(event);
        m_connection_with_clients.send_message(*send_event->message());
    }
}