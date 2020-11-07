#pragma once
#include <libipc/Encoder.h>
#include <libipc/ClientConnection.h>
#include <libipc/ServerConnection.h>
#include <libg/Rect.h>
#include <libg/String.h>
#include <malloc.h>

class GreetMessage : public Message {
public:
    GreetMessage(message_key_t key)
        : m_key(key)
    {
    }
    int id() const override { return 1; }
    int reply_id() const override { return 2; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        return buffer;
    }
private:
    message_key_t m_key;
};

class GreetMessageReply : public Message {
public:
    GreetMessageReply(message_key_t key,uint32_t connection_id)
        : m_key(key)
        , m_connection_id(connection_id)
    {
    }
    int id() const override { return 2; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t connection_id() const { return m_connection_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_connection_id);
        return buffer;
    }
private:
    message_key_t m_key;
    uint32_t m_connection_id;
};

class CreateWindowMessage : public Message {
public:
    CreateWindowMessage(message_key_t key,uint32_t width,uint32_t height,int buffer_id)
        : m_key(key)
        , m_width(width)
        , m_height(height)
        , m_buffer_id(buffer_id)
    {
    }
    int id() const override { return 3; }
    int reply_id() const override { return 4; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    int buffer_id() const { return m_buffer_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_width);
        Encoder::append(buffer, m_height);
        Encoder::append(buffer, m_buffer_id);
        return buffer;
    }
private:
    message_key_t m_key;
    uint32_t m_width;
    uint32_t m_height;
    int m_buffer_id;
};

class CreateWindowMessageReply : public Message {
public:
    CreateWindowMessageReply(message_key_t key,uint32_t window_id)
        : m_key(key)
        , m_window_id(window_id)
    {
    }
    int id() const override { return 4; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_window_id);
        return buffer;
    }
private:
    message_key_t m_key;
    uint32_t m_window_id;
};

class SetBufferMessage : public Message {
public:
    SetBufferMessage(message_key_t key,uint32_t window_id,int buffer_id)
        : m_key(key)
        , m_window_id(window_id)
        , m_buffer_id(buffer_id)
    {
    }
    int id() const override { return 5; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    int buffer_id() const { return m_buffer_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_window_id);
        Encoder::append(buffer, m_buffer_id);
        return buffer;
    }
private:
    message_key_t m_key;
    uint32_t m_window_id;
    int m_buffer_id;
};

class SetBarStyleMessage : public Message {
public:
    SetBarStyleMessage(message_key_t key,uint32_t window_id,uint32_t color,int text_style)
        : m_key(key)
        , m_window_id(window_id)
        , m_color(color)
        , m_text_style(text_style)
    {
    }
    int id() const override { return 6; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    uint32_t color() const { return m_color; }
    int text_style() const { return m_text_style; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_window_id);
        Encoder::append(buffer, m_color);
        Encoder::append(buffer, m_text_style);
        return buffer;
    }
private:
    message_key_t m_key;
    uint32_t m_window_id;
    uint32_t m_color;
    int m_text_style;
};

class SetTitleMessage : public Message {
public:
    SetTitleMessage(message_key_t key,uint32_t window_id,LG::String title)
        : m_key(key)
        , m_window_id(window_id)
        , m_title(title)
    {
    }
    int id() const override { return 7; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    LG::String title() const { return m_title; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_window_id);
        Encoder::append(buffer, m_title);
        return buffer;
    }
private:
    message_key_t m_key;
    uint32_t m_window_id;
    LG::String m_title;
};

class InvalidateMessage : public Message {
public:
    InvalidateMessage(message_key_t key,uint32_t window_id,LG::Rect rect)
        : m_key(key)
        , m_window_id(window_id)
        , m_rect(rect)
    {
    }
    int id() const override { return 8; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    LG::Rect rect() const { return m_rect; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_window_id);
        Encoder::append(buffer, m_rect);
        return buffer;
    }
private:
    message_key_t m_key;
    uint32_t m_window_id;
    LG::Rect m_rect;
};

class WindowServerDecoder : public MessageDecoder {
public:
    WindowServerDecoder() {}
    int magic() const { return 320; }
    UniquePtr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) override
    {
        int msg_id, decoder_magic;
        size_t saved_dml = decoded_msg_len;
        Encoder::decode(buf, decoded_msg_len, decoder_magic);
        Encoder::decode(buf, decoded_msg_len, msg_id);
        if (magic() != decoder_magic) {
            decoded_msg_len = saved_dml;
            return nullptr;
        }
        message_key_t secret_key;
        Encoder::decode(buf, decoded_msg_len, secret_key);

        uint32_t var_connection_id;
        uint32_t var_width;
        uint32_t var_height;
        int var_buffer_id;
        uint32_t var_window_id;
        uint32_t var_color;
        int var_text_style;
        LG::String var_title;
        LG::Rect var_rect;
        
        switch(msg_id) {
        case 1:
            return new GreetMessage(secret_key);
        case 2:
            Encoder::decode(buf, decoded_msg_len, var_connection_id);
            return new GreetMessageReply(secret_key, var_connection_id);
        case 3:
            Encoder::decode(buf, decoded_msg_len, var_width);
            Encoder::decode(buf, decoded_msg_len, var_height);
            Encoder::decode(buf, decoded_msg_len, var_buffer_id);
            return new CreateWindowMessage(secret_key, var_width, var_height, var_buffer_id);
        case 4:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            return new CreateWindowMessageReply(secret_key, var_window_id);
        case 5:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_buffer_id);
            return new SetBufferMessage(secret_key, var_window_id, var_buffer_id);
        case 6:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_color);
            Encoder::decode(buf, decoded_msg_len, var_text_style);
            return new SetBarStyleMessage(secret_key, var_window_id, var_color, var_text_style);
        case 7:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_title);
            return new SetTitleMessage(secret_key, var_window_id, var_title);
        case 8:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_rect);
            return new InvalidateMessage(secret_key, var_window_id, var_rect);
        default:
            decoded_msg_len = saved_dml;
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
        case 6:
            return handle(static_cast<const SetBarStyleMessage&>(msg));
        case 7:
            return handle(static_cast<const SetTitleMessage&>(msg));
        case 8:
            return handle(static_cast<const InvalidateMessage&>(msg));
        default:
            return nullptr;
        }
    }
    
    virtual UniquePtr<Message> handle(const GreetMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const CreateWindowMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const SetBufferMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const SetBarStyleMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const SetTitleMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const InvalidateMessage& msg) { return nullptr; }
};

class MouseMoveMessage : public Message {
public:
    MouseMoveMessage(message_key_t key,int win_id,uint32_t x,uint32_t y)
        : m_key(key)
        , m_win_id(win_id)
        , m_x(x)
        , m_y(y)
    {
    }
    int id() const override { return 1; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int win_id() const { return m_win_id; }
    uint32_t x() const { return m_x; }
    uint32_t y() const { return m_y; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_win_id);
        Encoder::append(buffer, m_x);
        Encoder::append(buffer, m_y);
        return buffer;
    }
private:
    message_key_t m_key;
    int m_win_id;
    uint32_t m_x;
    uint32_t m_y;
};

class MouseActionMessage : public Message {
public:
    MouseActionMessage(message_key_t key,int win_id,int type,uint32_t x,uint32_t y)
        : m_key(key)
        , m_win_id(win_id)
        , m_type(type)
        , m_x(x)
        , m_y(y)
    {
    }
    int id() const override { return 2; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int win_id() const { return m_win_id; }
    int type() const { return m_type; }
    uint32_t x() const { return m_x; }
    uint32_t y() const { return m_y; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_win_id);
        Encoder::append(buffer, m_type);
        Encoder::append(buffer, m_x);
        Encoder::append(buffer, m_y);
        return buffer;
    }
private:
    message_key_t m_key;
    int m_win_id;
    int m_type;
    uint32_t m_x;
    uint32_t m_y;
};

class MouseLeaveMessage : public Message {
public:
    MouseLeaveMessage(message_key_t key,int win_id,uint32_t x,uint32_t y)
        : m_key(key)
        , m_win_id(win_id)
        , m_x(x)
        , m_y(y)
    {
    }
    int id() const override { return 3; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int win_id() const { return m_win_id; }
    uint32_t x() const { return m_x; }
    uint32_t y() const { return m_y; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_win_id);
        Encoder::append(buffer, m_x);
        Encoder::append(buffer, m_y);
        return buffer;
    }
private:
    message_key_t m_key;
    int m_win_id;
    uint32_t m_x;
    uint32_t m_y;
};

class KeyboardMessage : public Message {
public:
    KeyboardMessage(message_key_t key,int win_id,uint32_t kbd_key)
        : m_key(key)
        , m_win_id(win_id)
        , m_kbd_key(kbd_key)
    {
    }
    int id() const override { return 4; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int win_id() const { return m_win_id; }
    uint32_t kbd_key() const { return m_kbd_key; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_win_id);
        Encoder::append(buffer, m_kbd_key);
        return buffer;
    }
private:
    message_key_t m_key;
    int m_win_id;
    uint32_t m_kbd_key;
};

class DisplayMessage : public Message {
public:
    DisplayMessage(message_key_t key,LG::Rect rect)
        : m_key(key)
        , m_rect(rect)
    {
    }
    int id() const override { return 5; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    LG::Rect rect() const { return m_rect; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_rect);
        return buffer;
    }
private:
    message_key_t m_key;
    LG::Rect m_rect;
};

class DisconnectMessage : public Message {
public:
    DisconnectMessage(message_key_t key,int reason)
        : m_key(key)
        , m_reason(reason)
    {
    }
    int id() const override { return 6; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int reason() const { return m_reason; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_reason);
        return buffer;
    }
private:
    message_key_t m_key;
    int m_reason;
};

class WindowClientDecoder : public MessageDecoder {
public:
    WindowClientDecoder() {}
    int magic() const { return 737; }
    UniquePtr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) override
    {
        int msg_id, decoder_magic;
        size_t saved_dml = decoded_msg_len;
        Encoder::decode(buf, decoded_msg_len, decoder_magic);
        Encoder::decode(buf, decoded_msg_len, msg_id);
        if (magic() != decoder_magic) {
            decoded_msg_len = saved_dml;
            return nullptr;
        }
        message_key_t secret_key;
        Encoder::decode(buf, decoded_msg_len, secret_key);

        int var_win_id;
        uint32_t var_x;
        uint32_t var_y;
        int var_type;
        uint32_t var_kbd_key;
        LG::Rect var_rect;
        int var_reason;
        
        switch(msg_id) {
        case 1:
            Encoder::decode(buf, decoded_msg_len, var_win_id);
            Encoder::decode(buf, decoded_msg_len, var_x);
            Encoder::decode(buf, decoded_msg_len, var_y);
            return new MouseMoveMessage(secret_key, var_win_id, var_x, var_y);
        case 2:
            Encoder::decode(buf, decoded_msg_len, var_win_id);
            Encoder::decode(buf, decoded_msg_len, var_type);
            Encoder::decode(buf, decoded_msg_len, var_x);
            Encoder::decode(buf, decoded_msg_len, var_y);
            return new MouseActionMessage(secret_key, var_win_id, var_type, var_x, var_y);
        case 3:
            Encoder::decode(buf, decoded_msg_len, var_win_id);
            Encoder::decode(buf, decoded_msg_len, var_x);
            Encoder::decode(buf, decoded_msg_len, var_y);
            return new MouseLeaveMessage(secret_key, var_win_id, var_x, var_y);
        case 4:
            Encoder::decode(buf, decoded_msg_len, var_win_id);
            Encoder::decode(buf, decoded_msg_len, var_kbd_key);
            return new KeyboardMessage(secret_key, var_win_id, var_kbd_key);
        case 5:
            Encoder::decode(buf, decoded_msg_len, var_rect);
            return new DisplayMessage(secret_key, var_rect);
        case 6:
            Encoder::decode(buf, decoded_msg_len, var_reason);
            return new DisconnectMessage(secret_key, var_reason);
        default:
            decoded_msg_len = saved_dml;
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
            return handle(static_cast<const MouseMoveMessage&>(msg));
        case 2:
            return handle(static_cast<const MouseActionMessage&>(msg));
        case 3:
            return handle(static_cast<const MouseLeaveMessage&>(msg));
        case 4:
            return handle(static_cast<const KeyboardMessage&>(msg));
        case 5:
            return handle(static_cast<const DisplayMessage&>(msg));
        case 6:
            return handle(static_cast<const DisconnectMessage&>(msg));
        default:
            return nullptr;
        }
    }
    
    virtual UniquePtr<Message> handle(const MouseMoveMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const MouseActionMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const MouseLeaveMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const KeyboardMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const DisplayMessage& msg) { return nullptr; }
    virtual UniquePtr<Message> handle(const DisconnectMessage& msg) { return nullptr; }
};

