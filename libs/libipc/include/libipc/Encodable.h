#pragma once
#include <libipc/Encoder.h>

template <typename T>
class Encodable {
public:
    virtual void encode(EncodedMessage& buf) const { }
};