#pragma once
#include <cstddef>

template <typename T>
class Decodable {
public:
    virtual void decode(const char* buf, size_t& offset) { }
};