#include <sysdep.h>
#include <unistd.h>

int fork()
{
    return DO_SYSCALL_0(SYS_FORK);
}

int execve(const char* path, char** argv, char** env)
{
    return DO_SYSCALL_3(SYS_EXEC, (int)path, (int)argv, (int)env);
}

int wait(int pid)
{
    return DO_SYSCALL_1(SYS_WAITPID, pid);
}

pid_t getpid()
{
    return (pid_t)DO_SYSCALL_0(SYS_GETPID);
}

int setpgid(pid_t pid, pid_t pgid)
{
    return DO_SYSCALL_2(SYS_SETPGID, pid, pgid);
}

pid_t getpgid(pid_t pid)
{
    return (pid_t)DO_SYSCALL_1(SYS_GETPGID, pid);
}