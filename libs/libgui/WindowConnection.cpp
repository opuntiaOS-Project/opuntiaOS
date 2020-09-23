#include <libgui/WindowConnection.h>
#include <libipc/ClientConnection.h>
#include <memory.h>
#include <malloc.h>
#include <syscalls.h>

namespace Window {

WindowConnection::WindowConnection(int connection_fd)
    : m_connection_fd(connection_fd)
    , m_server_decoder()
    , m_client_decoder()
    , m_connection_with_server(m_connection_fd, m_server_decoder, m_client_decoder)
{
    if (m_connection_fd > 0) {
        connect(m_connection_fd, "/win.sock", 9);
    }

    send_greeting();
}

void WindowConnection::send_greeting()
{
    ServerGreetMessageReply* resp_message = (ServerGreetMessageReply*)m_connection_with_server.send_sync(ServerGreetMessage()).release();
    char id = resp_message->client_id() + '0';
    write(1, &id, 1);
}

}