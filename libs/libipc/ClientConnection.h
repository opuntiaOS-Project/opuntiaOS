#pragma once
#include "Message.h"
#include "MessageDecoder.h"
#include <libcxx/std/vector.h>
#include <syscalls.h>

class ClientConnection {
public:
    ClientConnection(int sock_fd, MessageDecoder& server_decoder, MessageDecoder& client_decoder)
        : m_connectionFd(sock_fd)
        , m_server_decoder(server_decoder)
        , m_client_decoder(client_decoder)
        , m_messages()
    {
    }

    bool send_message(const Message& msg)
    {
        auto encoded_msg = msg.encode();
        int wrote = write(m_connectionFd, encoded_msg.data(), encoded_msg.size());
        return wrote = encoded_msg.size();
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
                if (m_messages[i]->id() == 1 + msg.id()) {
                    return move(m_messages[i]);
                }
            }
            pump_messages();
        }
    }

    void pump_messages()
    {
        char buf[512];
        int read_cnt = read(m_connectionFd, buf, sizeof(buf));
        if (!read_cnt) {
            return;
        }

        size_t msg_len = 0;
        for (size_t i = 0; i < read_cnt; i += msg_len) {
            if (auto response = m_client_decoder.decode((buf + i), read_cnt - i, msg_len)) {
                m_messages.push_back(move(response));
            } else if (auto response = m_server_decoder.decode((buf + i), read_cnt - i, msg_len)) {
                m_messages.push_back(move(response));
            }
        }
    }

private:
    int m_connectionFd;
    Vector<unique_ptr<Message>> m_messages;
    MessageDecoder& m_server_decoder;
    MessageDecoder& m_client_decoder;
};