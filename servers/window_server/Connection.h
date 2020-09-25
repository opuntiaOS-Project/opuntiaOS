#pragma once
#include "WSConnection.h"
#include "WSServerDecoder.h"
#include <libipc/ServerConnection.h>
#include <syscalls.h>

class Connection {
public:
    static Connection& the();
    Connection(int connection_fd);

    void listen()
    {
        while (1) {
            m_connection_with_clients.pump_messages();
        }
    }

private:
    int m_connection_fd;
    ServerConnection<WServerDecoder, WindowClientDecoder> m_connection_with_clients;
    WServerDecoder m_server_decoder;
    WindowClientDecoder m_client_decoder;
};