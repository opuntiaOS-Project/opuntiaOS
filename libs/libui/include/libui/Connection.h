#pragma once
#include <libipc/ClientConnection.h>
#include <libui/ClientDecoder.h>
#include <sys/types.h>

namespace UI {

class Window;

class Connection {
public:
    static Connection& the();
    explicit Connection(int connection_fd);

    void greeting();
    int new_window(const Window& window);
    void set_buffer(const Window& window);

    template <class T>
    inline std::unique_ptr<T> send_sync_message(const Message& msg) { return std::unique_ptr<T>(m_connection_with_server.send_sync(msg)); }
    inline bool send_async_message(const Message& msg) const { return m_connection_with_server.send_message(msg); }
    inline void listen() { m_connection_with_server.pump_messages(); }

    // We use connection id as an unique key.
    inline int key() const { return m_connection_id; }

private:
    void setup_listners();

    int m_connection_fd;
    int m_connection_id;
    ClientConnection<WindowServerDecoder, ClientDecoder> m_connection_with_server;
    WindowServerDecoder m_server_decoder;
    ClientDecoder m_client_decoder;
};

}