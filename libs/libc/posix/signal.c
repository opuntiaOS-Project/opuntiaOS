#include <signal.h>
#include <sysdep.h>

int sigaction(int signo, void* callback)
{
    int res = DO_SYSCALL_2(SYS_SIGACTION, signo, callback);
    RETURN_WITH_ERRNO(res, 0, -1);
}

int raise(int signo)
{
    int res = DO_SYSCALL_1(SYS_RAISE, signo);
    RETURN_WITH_ERRNO(res, 0, res);
}

int kill(pid_t pid, int signo)
{
    int res = DO_SYSCALL_2(SYS_KILL, pid, signo);
    RETURN_WITH_ERRNO(res, 0, -1);
}
