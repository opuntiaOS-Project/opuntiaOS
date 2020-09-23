#pragma once

#include <libipc/ClientConnection.h>
#include <libipc/ServerConnection.h>
#include <malloc.h>

class ServerGreetMessage final : public Message {
public:
    ServerGreetMessage() { }
    ~ServerGreetMessage() { }

    int decoder_magic() const override
    {
        return 0x6;
    }

    int id() const override
    {
        return 0x1;
    }

    EncodedMessage encode() const override
    {
        EncodedMessage it;
        it.push_back(decoder_magic());
        it.push_back(id());
        return it;
    }
};

class ServerGreetMessageReply final : public Message {
public:
    ServerGreetMessageReply(int cid)
        : m_client_id(cid)
    {
    }

    ~ServerGreetMessageReply() { }

    int decoder_magic() const override
    {
        return 0x6;
    }

    int id() const override
    {
        return 0x2;
    }

    int client_id() const
    {
        return m_client_id;
    }

    EncodedMessage encode() const override
    {
        EncodedMessage it;
        it.push_back(decoder_magic());
        it.push_back(id());
        it.push_back(client_id());
        return it;
    }

private:
    int m_client_id;
};

class ServerMessageDecoder final : public MessageDecoder {
public:
    ServerMessageDecoder() { }
    ~ServerMessageDecoder() { }

    int magic() override
    {
        return 0x6;
    }

    unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) override
    {
        int mes_magic = buf[0];
        if (mes_magic != magic()) {
            decoded_msg_len = 0;
            return nullptr;
        }
        int id = buf[1];
        decoded_msg_len = 2;
        if (id == 1) {
            return new ServerGreetMessage();
        }
        if (id == 2) {
            decoded_msg_len = 3;
            return new ServerGreetMessageReply(buf[2]);
        }
        decoded_msg_len = 0;
        return nullptr;
    }

    unique_ptr<Message> handle(const Message& msg) override
    {
        if (msg.id() == 1) {
            return new ServerGreetMessageReply(0x09);
        }
        if (msg.id() == 2) {
            return nullptr;
        }
        return nullptr;
    }
};

class ClientMessageDecoder : public MessageDecoder {
public:
    ClientMessageDecoder() { }
    ~ClientMessageDecoder() { }

    int magic() override
    {
        return 0x7;
    }

    unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len)
    {
        int mes_magic = buf[size];
        if (mes_magic == magic()) {
            decoded_msg_len = 0;
            return nullptr;
        }
        return nullptr;
    }

    unique_ptr<Message> handle(const Message& msg) override
    {
        return nullptr;
    }
};
