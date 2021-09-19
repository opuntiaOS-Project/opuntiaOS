#include <sched.h>
#include <sysdep.h>
#include <unistd.h>

void sched_yield()
{
    DO_SYSCALL_0(SYS_SCHED_YIELD);
}

int nice(int inc)
{
    int res = DO_SYSCALL_1(SYS_NICE, inc);
    RETURN_WITH_ERRNO(res, 0, -1);
}