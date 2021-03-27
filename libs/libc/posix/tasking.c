#include <sysdep.h>
#include <unistd.h>

int fork()
{
    return DO_SYSCALL_0(SYSFORK);
}

int execve(const char* path, char** argv, char** env)
{
    return DO_SYSCALL_3(SYSEXEC, (int)path, (int)argv, (int)env);
}

int wait(int pid)
{
    return DO_SYSCALL_1(SYSWAITPID, pid);
}

pid_t getpid()
{
    return (pid_t)DO_SYSCALL_0(SYSGETPID);
}

int setpgid(pid_t pid, pid_t pgid)
{
    return DO_SYSCALL_2(SYSSETPGID, pid, pgid);
}

pid_t getpgid(pid_t pid)
{
    return (pid_t)DO_SYSCALL_1(SYSGETPGID, pid);
}