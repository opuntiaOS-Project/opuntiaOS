/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_SYSCALLS_HANDLERS_H
#define _KERNEL_SYSCALLS_HANDLERS_H

#include <libkern/syscall_structs.h>
#include <libkern/types.h>
#include <platform/generic/tasking/trapframe.h>

#define ksys0(sysid) ksyscall_impl(sysid, 0, 0, 0, 0);
#define ksys1(sysid, a) ksyscall_impl(sysid, a, 0, 0, 0);
#define ksys2(sysid, a, b) ksyscall_impl(sysid, a, b, 0, 0);
#define ksys3(sysid, a, b, c) ksyscall_impl(sysid, a, b, c, 0);
#define ksys4(sysid, a, b, c, d) ksyscall_impl(sysid, a, b, c, d);
int ksyscall_impl(int sysid, int a, int b, int c, int d);

void sys_handler(trapframe_t* tf);
void sys_restart_syscall(trapframe_t* tf);
void sys_exit(trapframe_t* tf);
void sys_fork(trapframe_t* tf);
void sys_read(trapframe_t* tf);
void sys_write(trapframe_t* tf);
void sys_open(trapframe_t* tf);
void sys_close(trapframe_t* tf);
void sys_waitpid(trapframe_t* tf);
void sys_creat(trapframe_t* tf);
void sys_exec(trapframe_t* tf);
void sys_chdir(trapframe_t* tf);
void sys_sigaction(trapframe_t* tf);
void sys_sigreturn(trapframe_t* tf);
void sys_gettimeofday(trapframe_t* tf);
void sys_lseek(trapframe_t* tf);
void sys_getpid(trapframe_t* tf);
void sys_getuid(trapframe_t* tf);
void sys_setuid(trapframe_t* tf);
void sys_setgid(trapframe_t* tf);
void sys_setreuid(trapframe_t* tf);
void sys_setregid(trapframe_t* tf);
void sys_kill(trapframe_t* tf);
void sys_mkdir(trapframe_t* tf);
void sys_rmdir(trapframe_t* tf);
void sys_unlink(trapframe_t* tf);
void sys_mmap(trapframe_t* tf);
void sys_munmap(trapframe_t* tf);
void sys_socket(trapframe_t* tf);
void sys_bind(trapframe_t* tf);
void sys_connect(trapframe_t* tf);
void sys_getdents(trapframe_t* tf);
void sys_ioctl(trapframe_t* tf);
void sys_setpgid(trapframe_t* tf);
void sys_getpgid(trapframe_t* tf);
void sys_create_thread(trapframe_t* tf);
void sys_sleep(trapframe_t* tf);
void sys_select(trapframe_t* tf);
void sys_fstat(trapframe_t* tf);
void sys_fsync(trapframe_t* tf);
void sys_sched_yield(trapframe_t* tf);
void sys_uname(trapframe_t* tf);
void sys_clock_settime(trapframe_t* tf);
void sys_clock_gettime(trapframe_t* tf);
void sys_clock_getres(trapframe_t* tf);
void sys_nice(trapframe_t* tf);
void sys_shbuf_create(trapframe_t* tf);
void sys_shbuf_get(trapframe_t* tf);
void sys_shbuf_free(trapframe_t* tf);
void sys_ptrace(trapframe_t* tf);

void sys_none(trapframe_t* tf);

#endif // _KERNEL_SYSCALLS_HANDLERS_H
