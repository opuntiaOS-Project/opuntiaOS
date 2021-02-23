#pragma once
#include <std/String.h>
#include <syscalls.h>

class Dbg {
public:
    Dbg() = default;
    ~Dbg() = default;

    friend const Dbg& operator<<(const Dbg& os, const String& value);
    friend const Dbg& operator<<(const Dbg& os, int value);
    friend const Dbg& operator<<(const Dbg& stream, uint32_t value);
    friend const Dbg& operator<<(const Dbg& stream, unsigned long value);

private:
    int m_fd { 1 };
};

class DbgPid {
public:
    explicit DbgPid(int pid)
        : m_pid(pid)
    {
    }

    ~DbgPid() = default;

    friend const DbgPid& operator<<(const DbgPid& os, const String& value);
    friend const DbgPid& operator<<(const DbgPid& os, int value);
    friend const DbgPid& operator<<(const DbgPid& stream, uint32_t value);
    friend const DbgPid& operator<<(const DbgPid& stream, unsigned long value);

    inline int pid() const { return m_pid; }

private:
    int m_pid { 0 };
    int m_fd { 1 };
};