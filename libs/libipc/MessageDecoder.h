#pragma once
#include "Message.h"
#include <memory.h>

class MessageDecoder {
public:
    MessageDecoder() {}
    virtual ~MessageDecoder() {};

    virtual int magic() {}
    virtual Message* decode(const char* buf, size_t size, size_t& decoded_msg_len) {}
    virtual unique_ptr<Message> handle(Message&) {}
};