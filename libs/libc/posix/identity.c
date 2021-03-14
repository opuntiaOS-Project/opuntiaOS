#include <sysdep.h>
#include <unistd.h>

uid_t getuid()
{
    return (uid_t)DO_SYSCALL_0(SYSGETUID);
}

uid_t geteuid()
{
}