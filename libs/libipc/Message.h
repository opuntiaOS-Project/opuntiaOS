#pragma once
#include <libcxx/std/vector.h>
#include <sys/types.h>

typedef Vector<uint8_t> EncodedMessage;

class Message {
public:
    Message() {}
    virtual ~Message() = default;

    virtual int decoder_magic() const {}
    virtual int id() const {}
    virtual EncodedMessage encode() const {}

protected:
    
};