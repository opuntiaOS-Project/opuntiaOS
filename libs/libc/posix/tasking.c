#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysdep.h>
#include <unistd.h>

int fork()
{
    int res = DO_SYSCALL_0(SYS_FORK);
    RETURN_WITH_ERRNO(res, res, -1);
}

int execve(const char* path, char** argv, char** env)
{
    int res = DO_SYSCALL_3(SYS_EXECVE, (int)path, (int)argv, (int)env);
    RETURN_WITH_ERRNO(res, -1, -1);
}

int execvpe(const char* path, char** argv, char** envp)
{
    if (strchr(path, '/')) {
        return execve(path, argv, envp);
    }

    char* full_path = malloc(256);
    size_t namelen = strlen(path);
    char* env_path = getenv("PATH");
    if (!env_path) {
        // Get it from confstr
        env_path = "/bin:/usr/bin";
    }

    int len = 0;
    int cnt = 0;
    for (int i = 0; env_path[i]; i += len) {
        len = 0;
        while (env_path[i + len] && env_path[i + len] != ':') {
            len++;
        }

        memcpy(full_path, &env_path[i], len);
        full_path[len] = '/';
        memcpy(&full_path[len + 1], path, namelen + 1);

        int err = execve(full_path, argv, envp);
        if (env_path[i + len] == ':') {
            len++;
        }
    }

    free(full_path);
    return -ENOENT;
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