#include <sysdep.h>
#include <unistd.h>

ssize_t read(int fd, char* buf, size_t count)
{
    return (ssize_t)DO_SYSCALL_3(SYSREAD, (int)fd, (int)buf, (int)count);
}

ssize_t write(int fd, const void* buf, size_t count)
{
    return (ssize_t)DO_SYSCALL_3(SYSWRITE, (int)fd, (int)buf, (int)count);
}