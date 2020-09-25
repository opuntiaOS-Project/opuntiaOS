#pragma once
#include "../servers/window_server/WSConnection.h"
#include <libipc/ClientConnection.h>
#include <sys/types.h>

namespace Window {

class WindowConnection {
public:
    WindowConnection(int connection_fd);

    void send_greeting();

private:
    int m_connection_fd;
    ClientConnection<WindowServerDecoder, WindowClientDecoder> m_connection_with_server;
    WindowServerDecoder m_server_decoder;
    WindowClientDecoder m_client_decoder;
};

}