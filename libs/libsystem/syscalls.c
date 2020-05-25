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
    SYSOPEN,
    SYSCLOSE,
    SYSEXEC,
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
    // syscall(SYSEXTT, 6, 0, 0, 0, 0); // SYSPRINT
}

void exit(int ret_code)
{
    syscall(SYSEXTT, ret_code, 0, 0, 0, 0);
}

enum OPEN_MODE {
    O_RDONLY,
    O_WRONLY,
    O_RDWR
};
int open(const char *pathname, int flags)
{
    return syscall(SYSOPEN, (int)pathname, flags, 0, 0, 0);
}