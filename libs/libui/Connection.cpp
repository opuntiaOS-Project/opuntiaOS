#include "Connection.h"
#include "Window.h"
#include <libipc/ClientConnection.h>
#include <libui/Connection.h>
#include <malloc.h>
#include <memory.h>
#include <std/Dbg.h>
#include <syscalls.h>

// #define DEBUG_CONNECTION

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
    LFoundation::EventLoop::the().add(
        m_connection_fd, [] {
            Connection::the().listen();
        },
        nullptr);
}

void Connection::greeting()
{
    auto resp_message = send_sync_message<GreetMessageReply>(GreetMessage(getpid()));
    m_connection_id = resp_message->connection_id();
    m_connection_with_server.set_accepted_key(m_connection_id);
#ifdef DEBUG_CONNECTION
    Dbg() << "Got greet with server\n";
#endif
}

int Connection::new_window(const Window& window)
{
    auto message = CreateWindowMessage(key(), window.type(), window.bounds().width(), window.bounds().height(), window.buffer().id(), window.icon_path());
    auto resp_message = send_sync_message<CreateWindowMessageReply>(message);
#ifdef DEBUG_CONNECTION
    Dbg() << "New window created\n";
#endif
    return resp_message->window_id();
}
}