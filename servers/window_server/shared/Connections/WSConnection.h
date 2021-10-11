// Auto generated with utils/ConnectionCompiler
// See .ipc file

#pragma once
#include <libg/Rect.h>
#include <libg/string.h>
#include <libipc/ClientConnection.h>
#include <libipc/Encoder.h>
#include <libipc/ServerConnection.h>
#include <new>

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
    GreetMessageReply(message_key_t key, uint32_t connection_id)
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
    CreateWindowMessage(message_key_t key, int type, uint32_t width, uint32_t height, int buffer_id, LG::string title, LG::string icon_path, uint32_t color, uint32_t menubar_style)
        : m_key(key)
        , m_type(type)
        , m_width(width)
        , m_height(height)
        , m_buffer_id(buffer_id)
        , m_title(title)
        , m_icon_path(icon_path)
        , m_color(color)
        , m_menubar_style(menubar_style)
    {
    }
    int id() const override { return 3; }
    int reply_id() const override { return 4; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    int type() const { return m_type; }
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    int buffer_id() const { return m_buffer_id; }
    LG::string title() const { return m_title; }
    LG::string icon_path() const { return m_icon_path; }
    uint32_t color() const { return m_color; }
    uint32_t menubar_style() const { return m_menubar_style; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_type);
        Encoder::append(buffer, m_width);
        Encoder::append(buffer, m_height);
        Encoder::append(buffer, m_buffer_id);
        Encoder::append(buffer, m_title);
        Encoder::append(buffer, m_icon_path);
        Encoder::append(buffer, m_color);
        Encoder::append(buffer, m_menubar_style);
        return buffer;
    }

private:
    message_key_t m_key;
    int m_type;
    uint32_t m_width;
    uint32_t m_height;
    int m_buffer_id;
    LG::string m_title;
    LG::string m_icon_path;
    uint32_t m_color;
    uint32_t m_menubar_style;
};

class CreateWindowMessageReply : public Message {
public:
    CreateWindowMessageReply(message_key_t key, uint32_t window_id)
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

class DestroyWindowMessage : public Message {
public:
    DestroyWindowMessage(message_key_t key, uint32_t window_id)
        : m_key(key)
        , m_window_id(window_id)
    {
    }
    int id() const override { return 5; }
    int reply_id() const override { return 6; }
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

class DestroyWindowMessageReply : public Message {
public:
    DestroyWindowMessageReply(message_key_t key, uint32_t status)
        : m_key(key)
        , m_status(status)
    {
    }
    int id() const override { return 6; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t status() const { return m_status; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_status);
        return buffer;
    }

private:
    message_key_t m_key;
    uint32_t m_status;
};

class SetBufferMessage : public Message {
public:
    SetBufferMessage(message_key_t key, uint32_t window_id, int buffer_id, int format, LG::Rect bounds)
        : m_key(key)
        , m_window_id(window_id)
        , m_buffer_id(buffer_id)
        , m_format(format)
        , m_bounds(bounds)
    {
    }
    int id() const override { return 7; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    int buffer_id() const { return m_buffer_id; }
    int format() const { return m_format; }
    LG::Rect bounds() const { return m_bounds; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_window_id);
        Encoder::append(buffer, m_buffer_id);
        Encoder::append(buffer, m_format);
        Encoder::append(buffer, m_bounds);
        return buffer;
    }

private:
    message_key_t m_key;
    uint32_t m_window_id;
    int m_buffer_id;
    int m_format;
    LG::Rect m_bounds;
};

class SetBarStyleMessage : public Message {
public:
    SetBarStyleMessage(message_key_t key, uint32_t window_id, uint32_t color, uint32_t menubar_style)
        : m_key(key)
        , m_window_id(window_id)
        , m_color(color)
        , m_menubar_style(menubar_style)
    {
    }
    int id() const override { return 8; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    uint32_t color() const { return m_color; }
    uint32_t menubar_style() const { return m_menubar_style; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_window_id);
        Encoder::append(buffer, m_color);
        Encoder::append(buffer, m_menubar_style);
        return buffer;
    }

private:
    message_key_t m_key;
    uint32_t m_window_id;
    uint32_t m_color;
    uint32_t m_menubar_style;
};

class SetTitleMessage : public Message {
public:
    SetTitleMessage(message_key_t key, uint32_t window_id, LG::string title)
        : m_key(key)
        , m_window_id(window_id)
        , m_title(title)
    {
    }
    int id() const override { return 9; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    LG::string title() const { return m_title; }
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
    LG::string m_title;
};

class InvalidateMessage : public Message {
public:
    InvalidateMessage(message_key_t key, uint32_t window_id, LG::Rect rect)
        : m_key(key)
        , m_window_id(window_id)
        , m_rect(rect)
    {
    }
    int id() const override { return 10; }
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

class AskBringToFrontMessage : public Message {
public:
    AskBringToFrontMessage(message_key_t key, uint32_t window_id, uint32_t target_window_id)
        : m_key(key)
        , m_window_id(window_id)
        , m_target_window_id(target_window_id)
    {
    }
    int id() const override { return 11; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    uint32_t target_window_id() const { return m_target_window_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_window_id);
        Encoder::append(buffer, m_target_window_id);
        return buffer;
    }

private:
    message_key_t m_key;
    uint32_t m_window_id;
    uint32_t m_target_window_id;
};

class MenuBarCreateMenuMessage : public Message {
public:
    MenuBarCreateMenuMessage(message_key_t key, uint32_t window_id, LG::string title)
        : m_key(key)
        , m_window_id(window_id)
        , m_title(title)
    {
    }
    int id() const override { return 12; }
    int reply_id() const override { return 13; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    LG::string title() const { return m_title; }
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
    LG::string m_title;
};

class MenuBarCreateMenuMessageReply : public Message {
public:
    MenuBarCreateMenuMessageReply(message_key_t key, int status, uint32_t menu_id)
        : m_key(key)
        , m_status(status)
        , m_menu_id(menu_id)
    {
    }
    int id() const override { return 13; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    int status() const { return m_status; }
    uint32_t menu_id() const { return m_menu_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_status);
        Encoder::append(buffer, m_menu_id);
        return buffer;
    }

private:
    message_key_t m_key;
    int m_status;
    uint32_t m_menu_id;
};

class MenuBarCreateItemMessage : public Message {
public:
    MenuBarCreateItemMessage(message_key_t key, uint32_t window_id, uint32_t menu_id, int item_id, LG::string title)
        : m_key(key)
        , m_window_id(window_id)
        , m_menu_id(menu_id)
        , m_item_id(item_id)
        , m_title(title)
    {
    }
    int id() const override { return 14; }
    int reply_id() const override { return 15; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    uint32_t window_id() const { return m_window_id; }
    uint32_t menu_id() const { return m_menu_id; }
    int item_id() const { return m_item_id; }
    LG::string title() const { return m_title; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_window_id);
        Encoder::append(buffer, m_menu_id);
        Encoder::append(buffer, m_item_id);
        Encoder::append(buffer, m_title);
        return buffer;
    }

private:
    message_key_t m_key;
    uint32_t m_window_id;
    uint32_t m_menu_id;
    int m_item_id;
    LG::string m_title;
};

class MenuBarCreateItemMessageReply : public Message {
public:
    MenuBarCreateItemMessageReply(message_key_t key, int status)
        : m_key(key)
        , m_status(status)
    {
    }
    int id() const override { return 15; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 320; }
    int status() const { return m_status; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_status);
        return buffer;
    }

private:
    message_key_t m_key;
    int m_status;
};

class BaseWindowServerDecoder : public MessageDecoder {
public:
    BaseWindowServerDecoder() { }
    int magic() const { return 320; }
    std::unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) override
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
        int var_type;
        uint32_t var_width;
        uint32_t var_height;
        int var_buffer_id;
        LG::string var_title;
        LG::string var_icon_path;
        uint32_t var_color;
        uint32_t var_menubar_style;
        uint32_t var_window_id;
        uint32_t var_status;
        int var_format;
        LG::Rect var_bounds;
        LG::Rect var_rect;
        uint32_t var_target_window_id;
        uint32_t var_menu_id;
        int var_item_id;

        switch (msg_id) {
        case 1:
            return new GreetMessage(secret_key);
        case 2:
            Encoder::decode(buf, decoded_msg_len, var_connection_id);
            return new GreetMessageReply(secret_key, var_connection_id);
        case 3:
            Encoder::decode(buf, decoded_msg_len, var_type);
            Encoder::decode(buf, decoded_msg_len, var_width);
            Encoder::decode(buf, decoded_msg_len, var_height);
            Encoder::decode(buf, decoded_msg_len, var_buffer_id);
            Encoder::decode(buf, decoded_msg_len, var_title);
            Encoder::decode(buf, decoded_msg_len, var_icon_path);
            Encoder::decode(buf, decoded_msg_len, var_color);
            Encoder::decode(buf, decoded_msg_len, var_menubar_style);
            return new CreateWindowMessage(secret_key, var_type, var_width, var_height, var_buffer_id, var_title, var_icon_path, var_color, var_menubar_style);
        case 4:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            return new CreateWindowMessageReply(secret_key, var_window_id);
        case 5:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            return new DestroyWindowMessage(secret_key, var_window_id);
        case 6:
            Encoder::decode(buf, decoded_msg_len, var_status);
            return new DestroyWindowMessageReply(secret_key, var_status);
        case 7:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_buffer_id);
            Encoder::decode(buf, decoded_msg_len, var_format);
            Encoder::decode(buf, decoded_msg_len, var_bounds);
            return new SetBufferMessage(secret_key, var_window_id, var_buffer_id, var_format, var_bounds);
        case 8:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_color);
            Encoder::decode(buf, decoded_msg_len, var_menubar_style);
            return new SetBarStyleMessage(secret_key, var_window_id, var_color, var_menubar_style);
        case 9:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_title);
            return new SetTitleMessage(secret_key, var_window_id, var_title);
        case 10:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_rect);
            return new InvalidateMessage(secret_key, var_window_id, var_rect);
        case 11:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_target_window_id);
            return new AskBringToFrontMessage(secret_key, var_window_id, var_target_window_id);
        case 12:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_title);
            return new MenuBarCreateMenuMessage(secret_key, var_window_id, var_title);
        case 13:
            Encoder::decode(buf, decoded_msg_len, var_status);
            Encoder::decode(buf, decoded_msg_len, var_menu_id);
            return new MenuBarCreateMenuMessageReply(secret_key, var_status, var_menu_id);
        case 14:
            Encoder::decode(buf, decoded_msg_len, var_window_id);
            Encoder::decode(buf, decoded_msg_len, var_menu_id);
            Encoder::decode(buf, decoded_msg_len, var_item_id);
            Encoder::decode(buf, decoded_msg_len, var_title);
            return new MenuBarCreateItemMessage(secret_key, var_window_id, var_menu_id, var_item_id, var_title);
        case 15:
            Encoder::decode(buf, decoded_msg_len, var_status);
            return new MenuBarCreateItemMessageReply(secret_key, var_status);
        default:
            decoded_msg_len = saved_dml;
            return nullptr;
        }
    }

    std::unique_ptr<Message> handle(const Message& msg) override
    {
        if (magic() != msg.decoder_magic()) {
            return nullptr;
        }

        switch (msg.id()) {
        case 1:
            return handle(static_cast<const GreetMessage&>(msg));
        case 3:
            return handle(static_cast<const CreateWindowMessage&>(msg));
        case 5:
            return handle(static_cast<const DestroyWindowMessage&>(msg));
        case 7:
            return handle(static_cast<const SetBufferMessage&>(msg));
        case 8:
            return handle(static_cast<const SetBarStyleMessage&>(msg));
        case 9:
            return handle(static_cast<const SetTitleMessage&>(msg));
        case 10:
            return handle(static_cast<const InvalidateMessage&>(msg));
        case 11:
            return handle(static_cast<const AskBringToFrontMessage&>(msg));
        case 12:
            return handle(static_cast<const MenuBarCreateMenuMessage&>(msg));
        case 14:
            return handle(static_cast<const MenuBarCreateItemMessage&>(msg));
        default:
            return nullptr;
        }
    }

    virtual std::unique_ptr<Message> handle(const GreetMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const CreateWindowMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const DestroyWindowMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const SetBufferMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const SetBarStyleMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const SetTitleMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const InvalidateMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const AskBringToFrontMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const MenuBarCreateMenuMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const MenuBarCreateItemMessage& msg) { return nullptr; }
};

class MouseMoveMessage : public Message {
public:
    MouseMoveMessage(message_key_t key, int win_id, uint32_t x, uint32_t y)
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
    MouseActionMessage(message_key_t key, int win_id, int type, uint32_t x, uint32_t y)
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
    MouseLeaveMessage(message_key_t key, int win_id, uint32_t x, uint32_t y)
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

class MouseWheelMessage : public Message {
public:
    MouseWheelMessage(message_key_t key, int win_id, int wheel_data, uint32_t x, uint32_t y)
        : m_key(key)
        , m_win_id(win_id)
        , m_wheel_data(wheel_data)
        , m_x(x)
        , m_y(y)
    {
    }
    int id() const override { return 4; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int win_id() const { return m_win_id; }
    int wheel_data() const { return m_wheel_data; }
    uint32_t x() const { return m_x; }
    uint32_t y() const { return m_y; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_win_id);
        Encoder::append(buffer, m_wheel_data);
        Encoder::append(buffer, m_x);
        Encoder::append(buffer, m_y);
        return buffer;
    }

private:
    message_key_t m_key;
    int m_win_id;
    int m_wheel_data;
    uint32_t m_x;
    uint32_t m_y;
};

class KeyboardMessage : public Message {
public:
    KeyboardMessage(message_key_t key, int win_id, uint32_t kbd_key)
        : m_key(key)
        , m_win_id(win_id)
        , m_kbd_key(kbd_key)
    {
    }
    int id() const override { return 5; }
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
    DisplayMessage(message_key_t key, LG::Rect rect)
        : m_key(key)
        , m_rect(rect)
    {
    }
    int id() const override { return 6; }
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

class WindowCloseRequestMessage : public Message {
public:
    WindowCloseRequestMessage(message_key_t key, int win_id)
        : m_key(key)
        , m_win_id(win_id)
    {
    }
    int id() const override { return 7; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int win_id() const { return m_win_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_win_id);
        return buffer;
    }

private:
    message_key_t m_key;
    int m_win_id;
};

class ResizeMessage : public Message {
public:
    ResizeMessage(message_key_t key, int win_id, LG::Rect rect)
        : m_key(key)
        , m_win_id(win_id)
        , m_rect(rect)
    {
    }
    int id() const override { return 8; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int win_id() const { return m_win_id; }
    LG::Rect rect() const { return m_rect; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_win_id);
        Encoder::append(buffer, m_rect);
        return buffer;
    }

private:
    message_key_t m_key;
    int m_win_id;
    LG::Rect m_rect;
};

class DisconnectMessage : public Message {
public:
    DisconnectMessage(message_key_t key, int reason)
        : m_key(key)
        , m_reason(reason)
    {
    }
    int id() const override { return 9; }
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

class MenuBarActionMessage : public Message {
public:
    MenuBarActionMessage(message_key_t key, int win_id, int item_id)
        : m_key(key)
        , m_win_id(win_id)
        , m_item_id(item_id)
    {
    }
    int id() const override { return 10; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int win_id() const { return m_win_id; }
    int item_id() const { return m_item_id; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_win_id);
        Encoder::append(buffer, m_item_id);
        return buffer;
    }

private:
    message_key_t m_key;
    int m_win_id;
    int m_item_id;
};

class NotifyWindowStatusChangedMessage : public Message {
public:
    NotifyWindowStatusChangedMessage(message_key_t key, int win_id, int changed_window_id, int type)
        : m_key(key)
        , m_win_id(win_id)
        , m_changed_window_id(changed_window_id)
        , m_type(type)
    {
    }
    int id() const override { return 11; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int win_id() const { return m_win_id; }
    int changed_window_id() const { return m_changed_window_id; }
    int type() const { return m_type; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_win_id);
        Encoder::append(buffer, m_changed_window_id);
        Encoder::append(buffer, m_type);
        return buffer;
    }

private:
    message_key_t m_key;
    int m_win_id;
    int m_changed_window_id;
    int m_type;
};

class NotifyWindowIconChangedMessage : public Message {
public:
    NotifyWindowIconChangedMessage(message_key_t key, int win_id, int changed_window_id, LG::string icon_path)
        : m_key(key)
        , m_win_id(win_id)
        , m_changed_window_id(changed_window_id)
        , m_icon_path(icon_path)
    {
    }
    int id() const override { return 12; }
    int reply_id() const override { return -1; }
    int key() const override { return m_key; }
    int decoder_magic() const override { return 737; }
    int win_id() const { return m_win_id; }
    int changed_window_id() const { return m_changed_window_id; }
    LG::string icon_path() const { return m_icon_path; }
    EncodedMessage encode() const override
    {
        EncodedMessage buffer;
        Encoder::append(buffer, decoder_magic());
        Encoder::append(buffer, id());
        Encoder::append(buffer, key());
        Encoder::append(buffer, m_win_id);
        Encoder::append(buffer, m_changed_window_id);
        Encoder::append(buffer, m_icon_path);
        return buffer;
    }

private:
    message_key_t m_key;
    int m_win_id;
    int m_changed_window_id;
    LG::string m_icon_path;
};

class BaseWindowClientDecoder : public MessageDecoder {
public:
    BaseWindowClientDecoder() { }
    int magic() const { return 737; }
    std::unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) override
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
        int var_wheel_data;
        uint32_t var_kbd_key;
        LG::Rect var_rect;
        int var_reason;
        int var_item_id;
        int var_changed_window_id;
        LG::string var_icon_path;

        switch (msg_id) {
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
            Encoder::decode(buf, decoded_msg_len, var_wheel_data);
            Encoder::decode(buf, decoded_msg_len, var_x);
            Encoder::decode(buf, decoded_msg_len, var_y);
            return new MouseWheelMessage(secret_key, var_win_id, var_wheel_data, var_x, var_y);
        case 5:
            Encoder::decode(buf, decoded_msg_len, var_win_id);
            Encoder::decode(buf, decoded_msg_len, var_kbd_key);
            return new KeyboardMessage(secret_key, var_win_id, var_kbd_key);
        case 6:
            Encoder::decode(buf, decoded_msg_len, var_rect);
            return new DisplayMessage(secret_key, var_rect);
        case 7:
            Encoder::decode(buf, decoded_msg_len, var_win_id);
            return new WindowCloseRequestMessage(secret_key, var_win_id);
        case 8:
            Encoder::decode(buf, decoded_msg_len, var_win_id);
            Encoder::decode(buf, decoded_msg_len, var_rect);
            return new ResizeMessage(secret_key, var_win_id, var_rect);
        case 9:
            Encoder::decode(buf, decoded_msg_len, var_reason);
            return new DisconnectMessage(secret_key, var_reason);
        case 10:
            Encoder::decode(buf, decoded_msg_len, var_win_id);
            Encoder::decode(buf, decoded_msg_len, var_item_id);
            return new MenuBarActionMessage(secret_key, var_win_id, var_item_id);
        case 11:
            Encoder::decode(buf, decoded_msg_len, var_win_id);
            Encoder::decode(buf, decoded_msg_len, var_changed_window_id);
            Encoder::decode(buf, decoded_msg_len, var_type);
            return new NotifyWindowStatusChangedMessage(secret_key, var_win_id, var_changed_window_id, var_type);
        case 12:
            Encoder::decode(buf, decoded_msg_len, var_win_id);
            Encoder::decode(buf, decoded_msg_len, var_changed_window_id);
            Encoder::decode(buf, decoded_msg_len, var_icon_path);
            return new NotifyWindowIconChangedMessage(secret_key, var_win_id, var_changed_window_id, var_icon_path);
        default:
            decoded_msg_len = saved_dml;
            return nullptr;
        }
    }

    std::unique_ptr<Message> handle(const Message& msg) override
    {
        if (magic() != msg.decoder_magic()) {
            return nullptr;
        }

        switch (msg.id()) {
        case 1:
            return handle(static_cast<const MouseMoveMessage&>(msg));
        case 2:
            return handle(static_cast<const MouseActionMessage&>(msg));
        case 3:
            return handle(static_cast<const MouseLeaveMessage&>(msg));
        case 4:
            return handle(static_cast<const MouseWheelMessage&>(msg));
        case 5:
            return handle(static_cast<const KeyboardMessage&>(msg));
        case 6:
            return handle(static_cast<const DisplayMessage&>(msg));
        case 7:
            return handle(static_cast<const WindowCloseRequestMessage&>(msg));
        case 8:
            return handle(static_cast<const ResizeMessage&>(msg));
        case 9:
            return handle(static_cast<const DisconnectMessage&>(msg));
        case 10:
            return handle(static_cast<const MenuBarActionMessage&>(msg));
        case 11:
            return handle(static_cast<const NotifyWindowStatusChangedMessage&>(msg));
        case 12:
            return handle(static_cast<const NotifyWindowIconChangedMessage&>(msg));
        default:
            return nullptr;
        }
    }

    virtual std::unique_ptr<Message> handle(const MouseMoveMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const MouseActionMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const MouseLeaveMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const MouseWheelMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const KeyboardMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const DisplayMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const WindowCloseRequestMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const ResizeMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const DisconnectMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const MenuBarActionMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const NotifyWindowStatusChangedMessage& msg) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const NotifyWindowIconChangedMessage& msg) { return nullptr; }
};
