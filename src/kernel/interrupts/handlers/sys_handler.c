/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include <sys_handler.h>
#include <tasking/tasking.h>

/* 32 bit Linux-like syscalls */

static inline void set_return(trapframe_t* tf, int val)
{
    tf->eax = val;
}

void sys_handler(trapframe_t* tf)
{
    const void* syscalls[] = {
        sys_restart_syscall,
        sys_exit,
        sys_fork,
        sys_read,
        sys_open,
        sys_close,
        sys_exec,
    };
    void (*callee)(trapframe_t*) = (void*)syscalls[tf->eax];
    callee(tf);
}
void sys_restart_syscall(trapframe_t* tf)
{
    kprintd(tf->ebx);
}
void sys_exit(trapframe_t* tf)
{
    tasking_exit(tf);
}
void sys_fork(trapframe_t* tf)
{
    tasking_fork(tf);
}
void sys_read(trapframe_t* tf)
{
}
void sys_open(trapframe_t* tf)
{
}
void sys_close(trapframe_t* tf)
{
}
void sys_exec(trapframe_t* tf)
{
    tasking_exec(tf);
}
