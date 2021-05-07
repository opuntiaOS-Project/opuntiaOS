#include <sys/utsname.h>
#include <sysdep.h>

int uname(utsname_t* buf)
{
    int res = DO_SYSCALL_1(SYS_UNAME, buf);
    RETURN_WITH_ERRNO(res, 0, -1);
}