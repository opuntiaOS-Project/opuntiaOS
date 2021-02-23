#include <stdlib.h>
#include <sysdep.h>

void exit(int status)
{
    DO_SYSCALL_1(SYSEXIT, status);
}
