#include "Dbg.h"

const Dbg& operator<<(const Dbg& stream, const String& value)
{
    write(stream.m_fd, value.data(), value.size());
    return stream;
}

static void print_dec(int fd, uint32_t dec)
{
    int nxt = 0;
    char buf[16];

    while (dec > 0) {
        buf[nxt++] = (dec % 10) + '0';
        dec /= 10;
    }

    if (nxt == 0) {
        write(fd, "0", 1);
    }

    while (nxt) {
        write(fd, &buf[--nxt], 1);
    }
}

const Dbg& operator<<(const Dbg& stream, int value)
{
    if (value < 0) {
        write(stream.m_fd, "-", 1);
        value = -value;
    }
    print_dec(stream.m_fd, value);
    return stream;
}

const Dbg& operator<<(const Dbg& stream, uint32_t value)
{
    print_dec(stream.m_fd, value);
    return stream;
}

const Dbg& operator<<(const Dbg& stream, size_t value)
{
    print_dec(stream.m_fd, value);
    return stream;
}


// DbgPid

const DbgPid& operator<<(const DbgPid& stream, const String& value)
{
    if (getpid() == stream.pid())
        Dbg() << value;
    return stream;
}

const DbgPid& operator<<(const DbgPid& stream, int value)
{
    if (getpid() == stream.pid())
        Dbg() << value;
    return stream;
}

const DbgPid& operator<<(const DbgPid& stream, uint32_t value)
{
    if (getpid() == stream.pid())
        Dbg() << value;
    return stream;
}

const DbgPid& operator<<(const DbgPid& stream, size_t value)
{
    if (getpid() == stream.pid())
        Dbg() << value;
    return stream;
}
