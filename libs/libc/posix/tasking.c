#include <stdarg.h>
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

int execve(const char* path, char* const argv[], char* const envp[])
{
    int res = DO_SYSCALL_3(SYS_EXECVE, path, argv, envp);
    RETURN_WITH_ERRNO(res, -1, -1);
}

int execvpe(const char* path, char* const argv[], char* const envp[])
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

int execvp(const char* path, char* const argv[])
{
    return execvpe(path, argv, environ);
}

int execlp(const char* path, const char* arg0, ...)
{
    const char* args[16];
    int nxt = 0;
    args[nxt++] = arg0;

    va_list va;
    va_start(va, arg0);
    for (;;) {
        const char* arg = va_arg(va, const char*);
        if (!arg) {
            break;
        }
        args[nxt++] = arg;
    }
    va_end(va);
    args[nxt++] = NULL;
    return execvpe(path, (char* const*)args, environ);
}

int wait(int pid)
{
    int res = DO_SYSCALL_3(SYS_WAITPID, pid, NULL, 0);
    RETURN_WITH_ERRNO(res, pid, -1);
}

int waitpid(int pid, int* status, int options)
{
    int res = DO_SYSCALL_3(SYS_WAITPID, pid, status, options);
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

uint32_t sleep(uint32_t seconds)
{
    uint32_t res = DO_SYSCALL_1(SYS_NANOSLEEP, seconds);
    RETURN_WITH_ERRNO(res, seconds, 0);
}