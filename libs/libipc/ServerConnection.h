#pragma once
#include "Message.h"
#include "MessageDecoder.h"
#include <libcxx/std/vector.h>
#include <syscalls.h>

template<typename ServerDecoder, typename ClientDecoder>
class ServerConnection {
public:
    ServerConnection(int sock_fd, ServerDecoder& server_decoder, ClientDecoder& client_decoder)
        : m_connection_fd(sock_fd)
        , m_server_decoder(server_decoder)
        , m_client_decoder(client_decoder)
    {
    }

    bool send_message(const Message& msg)
    {
        auto encoded_msg = msg.encode();
        int wrote = write(m_connection_fd, encoded_msg.data(), encoded_msg.size());
        return wrote == encoded_msg.size();
    }

    void pump_messages()
    {
        char buf[512];
        int read_cnt = read(m_connection_fd, buf, sizeof(buf));
        if (read_cnt <= 0) {
            return;
        }

        size_t msg_len = 0;
        for (int i = 0; i < read_cnt; i += msg_len) {
            if (auto response = m_server_decoder.decode((buf + i), read_cnt - i, msg_len)) {
                if (auto answer = m_server_decoder.handle(*response)) {
                    send_message(*answer);
                }
            } else if (auto response = m_client_decoder.decode((buf + i), read_cnt - i, msg_len)) {
            }
        }
    }

private:
    int m_connection_fd;
    ServerDecoder& m_server_decoder;
    ClientDecoder& m_client_decoder;
};