/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "syscalls.h"

enum sysid {
    SYSPRINT = 0,
    SYSEXTT,
    SYSFORK,
    SYSREAD,
    SYSWRITE,
    SYSOPEN,
    SYSCLOSE,
    SYSWAITPID,
    SYSEXEC,
    SYSSIGACTION,
    SYSSIGRETURN,
    SYSRAISE,
};
typedef enum sysid sysid_t;

static inline int syscall(sysid_t sysid, int p1, int p2, int p3, int p4, int p5)
{
    int ret;
    asm volatile("push %%ebx;movl %2,%%ebx;int $0x80;pop %%ebx"
                 : "=a"(ret)
                 : "0"(sysid), "r"((int)(p1)), "c"((int)(p2)), "d"((int)(p3)), "S"((int)(p4)), "D"((int)(p5))
                 : "memory");
    return ret;
}

void print(int value)
{
    syscall(SYSPRINT, value, 0, 0, 0, 0); // SYSPRINT
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
    syscall(SYSEXTT, ret_code, 0, 0, 0, 0);
}

int open(const char *pathname, int flags)
{
    return syscall(SYSOPEN, (int)pathname, flags, 0, 0, 0);
}

int sigaction(int signo, void* callback)
{
    return syscall(SYSSIGACTION, (int)signo, (int)callback, 0, 0, 0);
}

int raise(int signo)
{
    return syscall(SYSRAISE, (int)signo, 0, 0, 0, 0);
}