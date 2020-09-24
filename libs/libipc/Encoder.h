#pragma once
#include <libcxx/std/vector.h>

typedef Vector<uint8_t> EncodedMessage;

class Encoder {
public:
    ~Encoder() { }

    static void append(EncodedMessage& buf, int val)
    {
        buf.push_back((uint8_t)val);
        buf.push_back((uint8_t)(val >> 8));
        buf.push_back((uint8_t)(val >> 16));
        buf.push_back((uint8_t)(val >> 24));
    }

    static void append(EncodedMessage& buf, uint32_t val)
    {
        buf.push_back((uint8_t)val);
        buf.push_back((uint8_t)(val >> 8));
        buf.push_back((uint8_t)(val >> 16));
        buf.push_back((uint8_t)(val >> 24));
    }

    static void decode(const char* buf, size_t& offset, uint32_t& val)
    {
        uint8_t b0 = buf[offset++];
        uint8_t b1 = buf[offset++];
        uint8_t b2 = buf[offset++];
        uint8_t b3 = buf[offset++];

        val = 0;
        val |= (uint8_t(b3) << 24);
        val |= (uint8_t(b2) << 16);
        val |= (uint8_t(b1) << 8);
        val |= (uint8_t(b0));
    }

    static void decode(const char* buf, size_t& offset, int& val)
    {
        uint8_t b0 = buf[offset++];
        uint8_t b1 = buf[offset++];
        uint8_t b2 = buf[offset++];
        uint8_t b3 = buf[offset++];

        val = 0;
        val |= (uint8_t(b3) << 24);
        val |= (uint8_t(b2) << 16);
        val |= (uint8_t(b1) << 8);
        val |= (uint8_t(b0));
    }

private:
    Encoder() { }
};