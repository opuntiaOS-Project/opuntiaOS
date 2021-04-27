#include <sched.h>
#include <sysdep.h>

void sched_yield()
{
    DO_SYSCALL_0(SYS_SCHEDYIELD);
}
