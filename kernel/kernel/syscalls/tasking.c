/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <libkern/time.h>
#include <platform/generic/syscalls/params.h>
#include <syscalls/handlers.h>
#include <syscalls/wrapper.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>

void sys_exit(trapframe_t* tf)
{
    tasking_exit((int)SYSCALL_VAR1(tf));
}

void sys_fork(trapframe_t* tf)
{
    tasking_fork();
}

void sys_waitpid(trapframe_t* tf)
{
    int __user* status = (int __user*)SYSCALL_VAR2(tf);
    int kstatus = 0;
    int ret = tasking_waitpid(SYSCALL_VAR1(tf), &kstatus, SYSCALL_VAR3(tf));
    umem_copy_to_user(status, &kstatus, sizeof(int));
    return_with_val(ret);
}

void sys_exec(trapframe_t* tf)
{
    int res = tasking_exec((char __user*)SYSCALL_VAR1(tf), (const char __user**)SYSCALL_VAR2(tf), (const char __user**)SYSCALL_VAR3(tf));
    if (res != 0) {
        return_with_val(res);
    }
}

void sys_sigaction(trapframe_t* tf)
{
    int res = signal_set_handler(RUNNING_THREAD, (int)SYSCALL_VAR1(tf), (uintptr_t)SYSCALL_VAR2(tf));
    return_with_val(-1);
}

void sys_sigreturn(trapframe_t* tf)
{
    signal_restore_thread_after_handling_signal(RUNNING_THREAD);
}

void sys_kill(trapframe_t* tf)
{
    thread_t* thread = thread_by_pid(SYSCALL_VAR1(tf));
    if (!thread) {
        return_with_val(-ESRCH);
    }
    int ret = tasking_signal(thread, SYSCALL_VAR2(tf));
    return_with_val(ret);
}

void sys_setpgid(trapframe_t* tf)
{
    pid_t pid, pgid;
    SYSCALL_INIT_VARS2(pid, pgid);

    proc_t* p = tasking_get_proc(pid);
    if (!p) {
        return_with_val(-ESRCH);
    }

    p->pgid = pgid;
    return_with_val(0);
}

void sys_getpgid(trapframe_t* tf)
{
    pid_t pid = SYSCALL_VAR1(tf);

    proc_t* p = tasking_get_proc(pid);
    if (!p) {
        return_with_val(-ESRCH);
    }

    return_with_val(p->pgid);
}

void sys_create_thread(trapframe_t* tf)
{
    proc_t* p = RUNNING_THREAD->process;
    thread_t* thread = proc_create_thread(p);
    if (!thread) {
        return_with_val(-EFAULT);
    }

    thread_create_params_t kparams;
    thread_create_params_t __user* params = (thread_create_params_t __user*)SYSCALL_VAR1(tf);
    umem_get_user(&kparams, params);
    set_instruction_pointer(thread->tf, kparams.entry_point);
    uintptr_t esp = kparams.stack_start + kparams.stack_size;
    set_stack_pointer(thread->tf, esp);
    set_frame_pointer(thread->tf, esp);

    return_with_val(thread->tid);
}

void sys_sched_yield(trapframe_t* tf)
{
    resched();
}

void sys_nice(trapframe_t* tf)
{
    int inc = SYSCALL_VAR1(tf);
    thread_t* thread = RUNNING_THREAD;
    if ((thread->process->prio + inc) < MAX_PRIO || (thread->process->prio + inc) > MIN_PRIO) {
        return_with_val(-1);
    }
    thread->process->prio += inc;
    return_with_val(0);
}