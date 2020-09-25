#pragma once
#include <libipc/Encoder.h>
#include <libipc/ClientConnection.h>
#include <libipc/ServerConnection.h>
#include <malloc.h>

class GreetMessage : public Message {
public:
    GreetMessage() {}
    int id() const override { return 1; }
    int decoder_magic() const override { return 130; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        return buffer;
    }
private:
};

class GreetMessageReply : public Message {
public:
    GreetMessageReply(uint32_t connection_id)
        : m_connection_id(connection_id)
    {
    }
    int id() const override { return 2; }
    int decoder_magic() const override { return 130; }
    uint32_t connection_id() const { return m_connection_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, m_connection_id);
        return buffer;
    }
private:
    uint32_t m_connection_id;
};

class WindowServerDecoder : public MessageDecoder {
public:
    WindowServerDecoder() {}
    int magic() const { return 130; }
    unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) override
    {
        int msg_id, decoder_magic;
        Encoder::decode(buf, decoded_msg_len, decoder_magic);
        Encoder::decode(buf, decoded_msg_len, msg_id);
        if (magic() != decoder_magic) {
            return nullptr;
        }
        
        switch(msg_id) {
        case 1:
            return new GreetMessage();
        case 2:
            uint32_t var_connection_id;
            Encoder::decode(buf, decoded_msg_len, var_connection_id);
            return new GreetMessageReply(var_connection_id);
        default:
            return nullptr;
        }
    }
    
    unique_ptr<Message> handle(const Message& msg) override
    {
        if (magic() != msg.decoder_magic()) {
            return nullptr;
        }
        
        switch(msg.id()) {
        case 1:
            return handle(static_cast<const GreetMessage&>(msg));
        case 2:
            return nullptr;
        default:
            return nullptr;
        }
    }
    
    virtual unique_ptr<Message> handle(const GreetMessage& msg) { return nullptr; }
};

class WindowClientDecoder : public MessageDecoder {
public:
    WindowClientDecoder() { }
    ~WindowClientDecoder() { }

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
