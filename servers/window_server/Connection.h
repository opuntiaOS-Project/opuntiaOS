#pragma once
#include "ServerMessages.h"
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
    ServerConnection m_connection_with_clients;
    ServerMessageDecoder m_server_decoder;
    ClientMessageDecoder m_client_decoder;
};