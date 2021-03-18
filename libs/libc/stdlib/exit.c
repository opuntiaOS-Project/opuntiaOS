#include <stdlib.h>
#include <sysdep.h>

void exit(int status)
{
    DO_SYSCALL_1(SYSEXIT, status);
    __builtin_unreachable();
}

void abort()
{
    // TODO: Raise signal
    exit(127);
}
