#pragma once

#include "Encoder.h"

template <typename T>
class Encodable {
public:
    virtual void encode(EncodedMessage& buf) const { }
};