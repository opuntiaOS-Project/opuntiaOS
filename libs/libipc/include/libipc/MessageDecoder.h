#pragma once
#include <libipc/Message.h>
#include <memory>

class MessageDecoder {
public:
    MessageDecoder() = default;
    virtual ~MessageDecoder() = default;

    virtual int magic() { return 0; }
    virtual std::unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) { return nullptr; }
    virtual std::unique_ptr<Message> handle(const Message&) { return nullptr; }
};