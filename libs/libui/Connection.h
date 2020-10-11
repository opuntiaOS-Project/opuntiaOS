#pragma once
#include "ClientDecoder.h"
#include "Window.h"
#include <libipc/ClientConnection.h>
#include <sys/types.h>

namespace UI {

class Connection {
public:
    static Connection& the();
    Connection(int connection_fd);

    void greeting();
    int new_window(const Window& window);
    void set_buffer(const Window& window);

    inline void listen() 
    {
        m_connection_with_server.pump_messages();
    }

private:
    int m_connection_fd;
    int m_connection_id;
    ClientConnection<WindowServerDecoder, ClientDecoder> m_connection_with_server;
    WindowServerDecoder m_server_decoder;
    ClientDecoder m_client_decoder;
};

}