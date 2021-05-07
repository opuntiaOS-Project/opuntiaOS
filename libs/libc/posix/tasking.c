#include <sysdep.h>
#include <unistd.h>

int fork()
{
    int res = DO_SYSCALL_0(SYS_FORK);
    RETURN_WITH_ERRNO(res, res, -1);
}

int execve(const char* path, char** argv, char** env)
{
    int res = DO_SYSCALL_3(SYS_EXEC, (int)path, (int)argv, (int)env);
    RETURN_WITH_ERRNO(res, -1, -1);
}

int wait(int pid)
{
    int res = DO_SYSCALL_1(SYS_WAITPID, pid);
    RETURN_WITH_ERRNO(res, pid, -1);
}

pid_t getpid()
{
    int res = DO_SYSCALL_0(SYS_GETPID);
    RETURN_WITH_ERRNO(res, (pid_t)res, -1);
}

int setpgid(pid_t pid, pid_t pgid)
{
    int res = DO_SYSCALL_2(SYS_SETPGID, pid, pgid);
    RETURN_WITH_ERRNO(res, 0, -1);
}

pid_t getpgid(pid_t pid)
{
    int res = DO_SYSCALL_1(SYS_GETPGID, pid);
    RETURN_WITH_ERRNO(res, (pid_t)res, -1);
}