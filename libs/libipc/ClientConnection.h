#pragma once
#include "Message.h"
#include "MessageDecoder.h"
#include <cassert.h>
#include <libfoundation/Event.h>
#include <libfoundation/EventLoop.h>
#include <libfoundation/EventReceiver.h>
#include <std/Dbg.h>
#include <std/Vector.h>
#include <syscalls.h>

template <typename ServerDecoder, typename ClientDecoder>
class ClientConnection : public LFoundation::EventReceiver {
public:
    ClientConnection(int sock_fd, ServerDecoder& server_decoder, ClientDecoder& client_decoder)
        : m_connection_fd(sock_fd)
        , m_server_decoder(server_decoder)
        , m_client_decoder(client_decoder)
        , m_messages()
    {
    }

    void set_accepted_key(int key) { m_accepted_key = key; }

    bool send_message(const Message& msg) const
    {
        auto encoded_msg = msg.encode();
        int wrote = write(m_connection_fd, encoded_msg.data(), encoded_msg.size());
        return wrote == encoded_msg.size();
    }

    UniquePtr<Message> send_sync(const Message& msg)
    {
        bool status = send_message(msg);
        return wait_for_answer(msg);
    }

    UniquePtr<Message> wait_for_answer(const Message& msg)
    {
        for (;;) {
            for (int i = 0; i < m_messages.size(); i++) {
                if (m_messages[i] && m_messages[i]->key() == msg.key() && m_messages[i]->id() == msg.reply_id()) {
                    return m_messages[i].release();
                }
            }
            pump_messages();
        }
    }

    void pump_messages()
    {
        Vector<char> buf;

        char tmpbuf[1024];

        int read_cnt;
        while ((read_cnt = read(m_connection_fd, tmpbuf, sizeof(tmpbuf)))) {
            if (read_cnt <= 0) {
                Dbg() << getpid() << " :: ClientConnection read error\n";
                return;
            }
            size_t buf_size = buf.size();
            buf.resize(buf_size + read_cnt);
            memcpy((uint8_t*)&buf.data()[buf_size], (uint8_t*)tmpbuf, read_cnt);
            if (read_cnt < sizeof(tmpbuf)) {
                break;
            }
        }

        size_t msg_len = 0;
        size_t buf_size = buf.size();
        for (size_t i = 0; i < buf_size; i += msg_len) {
            msg_len = 0;
            if (auto response = m_client_decoder.decode((buf.data() + i), read_cnt - i, msg_len)) {
                m_messages.push_back(move(response));
            } else if (auto response = m_server_decoder.decode((buf.data() + i), read_cnt - i, msg_len)) {
                m_messages.push_back(move(response));
            } else {
                Dbg() << getpid() << " :: ClientConnection read error\n";
                ASSERT_NOT_REACHED();
            }
        }

        if (m_messages.size() > 0) {
            // Note: We send an event to ourselves and use CallEvent to recognize the
            // event as sign to start processing of messages.
            LFoundation::EventLoop::the().add(*this, new LFoundation::CallEvent(nullptr));
        }
    }

    void receive_event(UniquePtr<LFoundation::Event> event) override
    {
        if (event->type() == LFoundation::Event::Type::DeferredInvoke) {
            // Note: The event was sent from pump_messages() and callback of CallEvent is 0!
            // Do NOT call callback here!
            auto msg = move(m_messages);
            for (int i = 0; i < msg.size(); i++) {
                if (msg[i] && msg[i]->decoder_magic() == m_client_decoder.magic() && msg[i]->key() == m_accepted_key) {
                    m_client_decoder.handle(*msg[i]);
                }
            }
        }
    }

private:
    int m_accepted_key { -1 };
    int m_connection_fd;
    Vector<UniquePtr<Message>> m_messages;
    ServerDecoder& m_server_decoder;
    ClientDecoder& m_client_decoder;
};