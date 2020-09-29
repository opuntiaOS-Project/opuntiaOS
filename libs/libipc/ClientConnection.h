#pragma once
#include "Message.h"
#include "MessageDecoder.h"
#include <libcxx/std/vector.h>
#include <syscalls.h>

template<typename ServerDecoder, typename ClientDecoder>
class ClientConnection {
public:
    ClientConnection(int sock_fd, ServerDecoder& server_decoder, ClientDecoder& client_decoder)
        : m_connection_fd(sock_fd)
        , m_server_decoder(server_decoder)
        , m_client_decoder(client_decoder)
        , m_messages()
    {
    }

    bool send_message(const Message& msg)
    {
        auto encoded_msg = msg.encode();
        int wrote = write(m_connection_fd, encoded_msg.data(), encoded_msg.size());
        return wrote == encoded_msg.size();
    }

    unique_ptr<Message> send_sync(const Message& msg)
    {
        bool status = send_message(msg);
        return wait_for_answer(msg);
    }

    unique_ptr<Message> wait_for_answer(const Message& msg)
    {
        for (;;) {
            for (int i = 0; i < m_messages.size(); i++) {
                if (m_messages[i] && m_messages[i]->id() == 1 + msg.id() && m_messages[i]->id() % 2 == 0) {
                    return m_messages[i].release();
                }
            }
            pump_messages();
        }
    }

    void pump_messages()
    {
        char buf[512];
        int read_cnt = read(m_connection_fd, buf, sizeof(buf));
        if (!read_cnt) {
            return;
        }

        size_t msg_len = 0;
        for (size_t i = 0; i < read_cnt; i += msg_len) {
            msg_len = 0;
            if (auto response = m_client_decoder.decode((buf + i), read_cnt - i, msg_len)) {
                m_messages.push_back(move(response));
            } else if (auto response = m_server_decoder.decode((buf + i), read_cnt - i, msg_len)) {
                m_messages.push_back(move(response));
            }
        }
    }

private:
    int m_connection_fd;
    Vector<unique_ptr<Message>> m_messages;
    ServerDecoder& m_server_decoder;
    ClientDecoder& m_client_decoder;
};