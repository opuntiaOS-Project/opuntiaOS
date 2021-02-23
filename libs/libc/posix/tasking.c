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