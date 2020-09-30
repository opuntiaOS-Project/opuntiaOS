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

class CreateWindowMessage : public Message {
public:
    CreateWindowMessage(uint32_t width,uint32_t height,int buffer_id)
        : m_width(width)
        , m_height(height)
        , m_buffer_id(buffer_id)
    {
    }
    int id() const override { return 3; }
    int decoder_magic() const override { return 130; }
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    int buffer_id() const { return m_buffer_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, m_width);
        Encoder::append(buffer, m_height);
        Encoder::append(buffer, m_buffer_id);
        return buffer;
    }
private:
    uint32_t m_width;
    uint32_t m_height;
    int m_buffer_id;
};

class CreateWindowMessageReply : public Message {
public:
    CreateWindowMessageReply(uint32_t window_id)
        : m_window_id(window_id)
    {
    }
    int id() const override { return 4; }
    int decoder_magic() const override { return 130; }
    uint32_t window_id() const { return m_window_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, m_window_id);
        return buffer;
    }
private:
    uint32_t m_window_id;
};

class SetBufferMessage : public Message {
public:
    SetBufferMessage(uint32_t windows_id,int buffer_id)
        : m_windows_id(windows_id)
        , m_buffer_id(buffer_id)
    {
    }
    int id() const override { return 5; }
    int decoder_magic() const override { return 130; }
    uint32_t windows_id() const { return m_windows_id; }
    int buffer_id() const { return m_buffer_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, m_windows_id);
        Encoder::append(buffer, m_buffer_id);
        return buffer;
    }
private:
    uint32_t m_windows_id;
    int m_buffer_id;
};

class WindowServerDecoder : public MessageDecoder {
public:
    WindowServerDecoder() {}
    int magic() const { return 130; }
    UniquePtr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) override
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
        case 3:
            uint32_t var_width;
            uint32_t var_height;
            int var_buffer_id;
            Encoder::decode(buf, decoded_msg_len, var_width);
            Encoder::decode(buf, decoded_msg_len, var_height);
            Encoder::decode(buf, decoded_msg_len, var_buffer_id);
            return new CreateWindowMessage(var_width, var_height, var_buffer_id);
        case 4:
            uint32_t var_window_id;
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            return new CreateWindowMessageReply(var_window_id);
        case 5:
            uint32_t var_windows_id;
            Encoder::decode(buf, decoded_msg_len, var_windows_id);
            Encoder::decode(buf, decoded_msg_len, var_buffer_id);
            return new SetBufferMessage(var_windows_id, var_buffer_id);
        default:
            return nullptr;
        }
    }
    
    UniquePtr<Message> handle(const Message& msg) override
    {
        if (magic() != msg.decoder_magic()) {
            return nullptr;
        }
        
        switch(msg.id()) {
        case 1:
            return handle(static_cast<const GreetMessage&>(msg));
        case 3:
            return handle(static_cast<const CreateWindowMessage&>(msg));
        case 5:
            return handle(static_cast<const SetBufferMessage&>(msg));
        default:
            return nullptr;
        }
    }
    
    virtual UniquePtr<Message> handle(const GreetMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const CreateWindowMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const SetBufferMessage& msg) { return nullptr; }
};

class WindowClientDecoder : public MessageDecoder {
public:
    WindowClientDecoder() { }
    ~WindowClientDecoder() { }

    int magic() override
    {
        return 0x7;
    }

    UniquePtr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len)
    {
        int mes_magic = buf[size];
        if (mes_magic == magic()) {
            decoded_msg_len = 0;
            return nullptr;
        }
        return nullptr;
    }

    UniquePtr<Message> handle(const Message& msg) override
    {
        return nullptr;
    }
};
