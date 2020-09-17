/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <syscalls.h>

int errno;

static inline int syscall(sysid_t sysid, int p1, int p2, int p3, int p4, int p5)
{
    int ret;
    asm volatile("push %%ebx;movl %2,%%ebx;int $0x80;pop %%ebx"
                 : "=a"(ret)
                 : "0"(sysid), "r"((int)(p1)), "c"((int)(p2)), "d"((int)(p3)), "S"((int)(p4)), "D"((int)(p5))
                 : "memory");
    return ret;
}

int read(int fd, char *buf, size_t count)
{
    return syscall(SYSREAD, (int)fd, (int)buf, (int)count, 0, 0);
}

int write(int fd, const void *buf, size_t count)
{
    return syscall(SYSWRITE, (int)fd, (int)buf, (int)count, 0, 0);
}

void exit(int ret_code)
{
    syscall(SYSEXIT, ret_code, 0, 0, 0, 0);
}

int fork()
{
    return syscall(SYSFORK, 0, 0, 0, 0, 0);
}

int wait(int pid)
{
    return syscall(SYSWAITPID, pid, 0, 0, 0, 0);
}

int execve(char* path, char** argv, char** env)
{
    return syscall(SYSEXEC, (int)path, (int)argv, (int)env, 0, 0);
}

int chdir(char* path)
{
    return syscall(SYSCHDIR, (int)path, 0, 0, 0, 0);
}

int mkdir(char* path)
{
    return syscall(SYSMKDIR, (int)path, 0, 0, 0, 0);
}

int rmdir(char* path)
{
    return syscall(SYSRMDIR, (int)path, 0, 0, 0, 0);
}

int unlink(char* path)
{
    return syscall(SYSUNLINK, (int)path, 0, 0, 0, 0);
}

int creat(char* path, uint32_t mode)
{
    return syscall(SYSCREAT, (int)path, (int)mode, 0, 0, 0);
}

int open(const char *pathname, int flags)
{
    return syscall(SYSOPEN, (int)pathname, flags, 0, 0, 0);
}

int close(int fd)
{
    return syscall(SYSCLOSE, (int)fd, 0, 0, 0, 0);
}

int sigaction(int signo, void* callback)
{
    return syscall(SYSSIGACTION, (int)signo, (int)callback, 0, 0, 0);
}

int raise(int signo)
{
    return syscall(SYSRAISE, (int)signo, 0, 0, 0, 0);
}

int kill(pid_t pid, int signo)
{
    return syscall(SYSKILL, (int)pid, (int)signo, 0, 0, 0);
}

int mmap(mmap_params_t* params)
{
    return syscall(SYSMMAP, (int)params, 0, 0, 0, 0);
}

int socket(int domain, int type, int protocol)
{
    return syscall(SYSSOCKET, (int)domain, (int)type, (int)protocol, 0, 0);
}

int bind(int sockfd, char* name, int len)
{
    return syscall(SYSBIND, (int)sockfd, (int)name, (int)len, 0, 0);
}

int connect(int sockfd, char* name, int len)
{
    return syscall(SYSCONNECT, (int)sockfd, (int)name, (int)len, 0, 0);
}

int getdents(int fd, char* buf, int len)
{
    return syscall(SYSGETDENTS, (int)fd, (int)buf, (int)len, 0, 0);
}

int ioctl(int fd, uint32_t cmd, uint32_t arg)
{
    return syscall(SYSIOCTL, (int)fd, (int)cmd, (int)arg, 0, 0);
}

int lseek(int fd, uint32_t off, int whence)
{
    return syscall(SYSLSEEK, (int)fd, (int)off, (int)whence, 0, 0);
}

pid_t getpid()
{
    return syscall(SYSGETPID, 0, 0, 0, 0, 0);
}

int setpgid(pid_t pid, pid_t pgid)
{
    return syscall(SYSSETPGID, (int)pid, (int)pgid, 0, 0, 0);
}

pid_t getpgid(pid_t pid)
{
    return syscall(SYSGETPGID, (int)pid, 0, 0, 0, 0);
}

int system_pthread_create(thread_create_params_t* params)
{
    return syscall(SYSPTHREADCREATE, (int)params, 0, 0, 0, 0);
}