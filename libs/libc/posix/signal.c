#include <signal.h>
#include <sysdep.h>
#include <unistd.h>

int sigaction(int signo, void* callback)
{
    int res = DO_SYSCALL_2(SYS_SIGACTION, signo, callback);
    RETURN_WITH_ERRNO(res, 0, -1);
}

int raise(int signo)
{
    return kill(getpid(), signo);
}

int kill(pid_t pid, int signo)
{
    int res = DO_SYSCALL_2(SYS_KILL, pid, signo);
    RETURN_WITH_ERRNO(res, 0, -1);
}
