#pragma once
#include "Window.h"
#include "../servers/window_server/WSConnection.h"
#include <libipc/ClientConnection.h>
#include <sys/types.h>

namespace Window {

class WindowConnection {
public:
    WindowConnection(int connection_fd);

    void greeting();
    int new_window(const Window& window);
    void set_buffer(const Window& window);

    static WindowConnection& the();
private:
    int m_connection_fd;
    int m_connection_id;
    ClientConnection<WindowServerDecoder, WindowClientDecoder> m_connection_with_server;
    WindowServerDecoder m_server_decoder;
    WindowClientDecoder m_client_decoder;
};

}