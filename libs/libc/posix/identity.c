#include <sysdep.h>
#include <unistd.h>

uid_t getuid()
{
    return (uid_t)DO_SYSCALL_0(SYS_GETUID);
}

uid_t geteuid()
{
}