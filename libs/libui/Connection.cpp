#include "Window.h"
#include <libui/Connection.h>
#include <libipc/ClientConnection.h>
#include <malloc.h>
#include <memory.h>
#include <syscalls.h>

namespace UI {

static Connection* s_the = 0;

Connection& Connection::the()
{
    // FIXME: Thread-safe method to be applied
    if (!s_the) {
        new Connection(socket(PF_LOCAL, 0, 0));
    }
    return *s_the;
}

Connection::Connection(int connection_fd)
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
    LFoundation::EventLoop::the().add(m_connection_fd, [] {
        Connection::the().listen();
    }, nullptr);
}

void Connection::greeting()
{
    GreetMessageReply* resp_message = (GreetMessageReply*)m_connection_with_server.send_sync(GreetMessage()).release();
    m_connection_id = resp_message->connection_id();
    write(1, "Got greet", 9);
}

int Connection::new_window(const Window& window)
{
    write(1, "Sending msg", 11);
    CreateWindowMessageReply* resp_message = (CreateWindowMessageReply*)m_connection_with_server.send_sync(CreateWindowMessage(window.bounds().width(), window.bounds().height(), window.buffer().id())).release();
    return resp_message->window_id();
}

void Connection::set_buffer(const Window& window)
{
    m_connection_with_server.send_message(SetBufferMessage(window.id(), window.buffer().id()));
}

}