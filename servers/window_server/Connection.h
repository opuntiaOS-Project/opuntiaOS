/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "WSConnection.h"
#include "WSServerDecoder.h"
#include <libipc/ServerConnection.h>
#include <syscalls.h>

class Connection {
public:
    static Connection& the();
    Connection(int connection_fd);

    inline void listen()
    {
        m_connection_with_clients.pump_messages();
    }

private:
    int m_connection_fd;
    ServerConnection<WServerDecoder, WindowClientDecoder> m_connection_with_clients;
    WServerDecoder m_server_decoder;
    WindowClientDecoder m_client_decoder;
};