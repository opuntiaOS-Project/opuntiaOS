#pragma once
#include <std/Vector.h>
#include <sys/types.h>

typedef Vector<uint8_t> EncodedMessage;

class Message {
public:
    Message() { }
    virtual ~Message() = default;

    virtual int decoder_magic() const { return 0; }
    virtual int id() const { return 0; }
    virtual EncodedMessage encode() const { return Vector<uint8_t>(); }
};