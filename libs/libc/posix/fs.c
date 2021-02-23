#include <sysdep.h>
#include <unistd.h>

int read(int fd, char* buf, size_t count)
{
    return DO_SYSCALL_3(SYSREAD, (int)fd, (int)buf, (int)count);
}

int write(int fd, const void* buf, size_t count)
{
    return DO_SYSCALL_3(SYSWRITE, (int)fd, (int)buf, (int)count);
}