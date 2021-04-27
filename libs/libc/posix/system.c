#include <sys/utsname.h>
#include <sysdep.h>

int uname(utsname_t* buf)
{
    return DO_SYSCALL_1(SYS_UNAME, buf);
}