#include "Window.h"
#include <libgui/WindowConnection.h>
#include <libipc/ClientConnection.h>
#include <malloc.h>
#include <memory.h>
#include <syscalls.h>

namespace Window {

static WindowConnection* s_the = 0;

WindowConnection& WindowConnection::the()
{
    // FIXME: Thread-safe method to be applied
    if (!s_the) {
        new WindowConnection(socket(PF_LOCAL, 0, 0));
    }
    return *s_the;
}

WindowConnection::WindowConnection(int connection_fd)
    : m_connection_fd(connection_fd)
    , m_server_decoder()
    , m_client_decoder()
    , m_connection_with_server(m_connection_fd, m_server_decoder, m_client_decoder)
{
    s_the = this;
    if (m_connection_fd > 0) {
        connect(m_connection_fd, "/win.sock", 9);
    }
    greeting();
}

void WindowConnection::greeting()
{
    GreetMessageReply* resp_message = (GreetMessageReply*)m_connection_with_server.send_sync(GreetMessage()).release();
    m_connection_id = resp_message->connection_id();
}

int WindowConnection::new_window(const Window& window)
{
    CreateWindowMessageReply* resp_message = (CreateWindowMessageReply*)m_connection_with_server.send_sync(CreateWindowMessage(window.width(), window.height())).release();
    return resp_message->window_id();
}

void WindowConnection::set_buffer(const Window& window)
{
    m_connection_with_server.send_message(SetBufferMessage(window.id(), window.buffer().id()));
}

}