/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Connection.h"

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
}
