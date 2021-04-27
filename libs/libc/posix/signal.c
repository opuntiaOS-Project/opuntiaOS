#include <signal.h>
#include <sysdep.h>

int sigaction(int signo, void* callback)
{
    return DO_SYSCALL_2(SYS_SIGACTION, signo, callback);
}

int raise(int signo)
{
    return DO_SYSCALL_1(SYS_RAISE, signo);
}

int kill(pid_t pid, int signo)
{
    return DO_SYSCALL_2(SYS_KILL, pid, signo);
}
