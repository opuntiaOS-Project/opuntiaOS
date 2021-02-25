#pragma once
#include <libipc/Message.h>
#include <libipc/MessageDecoder.h>
#include <std/Dbg.h>
#include <std/Vector.h>

template <typename ServerDecoder, typename ClientDecoder>
class ServerConnection {
public:
    ServerConnection(int sock_fd, ServerDecoder& server_decoder, ClientDecoder& client_decoder)
        : m_connection_fd(sock_fd)
        , m_server_decoder(server_decoder)
        , m_client_decoder(client_decoder)
    {
    }

    bool send_message(const Message& msg) const
    {
        auto encoded_msg = msg.encode();
        int wrote = write(m_connection_fd, encoded_msg.data(), encoded_msg.size());
        return wrote == encoded_msg.size();
    }

    void pump_messages()
    {
        Vector<char> buf;

        char tmpbuf[1024];

        int read_cnt;
        while ((read_cnt = read(m_connection_fd, tmpbuf, sizeof(tmpbuf)))) {
            if (read_cnt <= 0) {
                Dbg() << getpid() << " :: ServerConnection read error\n";
                return;
            }
            size_t buf_size = buf.size();
            buf.resize(buf_size + read_cnt);
            memcpy(&buf.data()[buf_size], tmpbuf, read_cnt);
            if (read_cnt < sizeof(tmpbuf)) {
                break;
            }
        }

        size_t msg_len = 0;
        size_t buf_size = buf.size();
        for (int i = 0; i < buf_size; i += msg_len) {
            msg_len = 0;
            if (auto response = m_server_decoder.decode((buf.data() + i), read_cnt - i, msg_len)) {
                if (auto answer = m_server_decoder.handle(*response)) {
                    send_message(*answer);
                }
            } else if (auto response = m_client_decoder.decode((buf.data() + i), read_cnt - i, msg_len)) {

            } else {
                ASSERT_NOT_REACHED();
            }
        }
    }

private:
    int m_connection_fd;
    ServerDecoder& m_server_decoder;
    ClientDecoder& m_client_decoder;
};