#include <signal.h>
#include <sysdep.h>

int sigaction(int signo, void* callback)
{
    return DO_SYSCALL_2(SYSSIGACTION, signo, callback);
}

int raise(int signo)
{
    return DO_SYSCALL_1(SYSRAISE, signo);
}

int kill(pid_t pid, int signo)
{
    return DO_SYSCALL_2(SYSRAISE, pid, signo);
}
