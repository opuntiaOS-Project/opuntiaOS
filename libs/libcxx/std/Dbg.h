#pragma once
#include <libcxx/std/String.h>
#include <libcxx/syscalls.h>

class Dbg {
public:
    Dbg() = default;
    ~Dbg() { }

    friend const Dbg& operator<<(const Dbg& os, const String& value);
    friend const Dbg& operator<<(const Dbg& os, int value);
    friend const Dbg& operator<<(const Dbg& stream, uint32_t value);
    friend const Dbg& operator<<(const Dbg& stream, size_t value);
private:
    int m_fd { 1 };
};