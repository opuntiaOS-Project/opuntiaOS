#pragma once
#include "../servers/window_server/ServerMessages.h"
#include <libipc/ClientConnection.h>
#include <sys/types.h>

namespace Window {

class WindowConnection {
public:
    WindowConnection(int connection_fd);

    void send_greeting();

private:
    int m_connection_fd;
    ClientConnection m_connection_with_server;
    ServerMessageDecoder m_server_decoder;
    ClientMessageDecoder m_client_decoder;
};

}