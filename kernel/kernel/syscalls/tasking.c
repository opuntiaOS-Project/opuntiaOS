/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <platform/generic/syscalls/params.h>
#include <syscalls/handlers.h>
#include <tasking/sched.h>
#include <tasking/tasking.h>

void sys_exit(trapframe_t* tf)
{
    tasking_exit((int)param1);
}

void sys_fork(trapframe_t* tf)
{
    tasking_fork(tf);
}

void sys_waitpid(trapframe_t* tf)
{
    int ret = tasking_waitpid(param1);
    return_with_val(ret);
}

void sys_exec(trapframe_t* tf)
{
    int res = tasking_exec((char*)param1, (const char**)param2, (const char**)param3);
    if (res != 0) {
        return_with_val(res);
    }
}

void sys_sigaction(trapframe_t* tf)
{
    int res = signal_set_handler(RUNNING_THREAD, (int)param1, (void*)param2);
    return_with_val(res);
}

void sys_sigreturn(trapframe_t* tf)
{
    signal_restore_thread_after_handling_signal(RUNNING_THREAD);
}

void sys_kill(trapframe_t* tf)
{
    thread_t* thread = thread_by_pid(param1);
    int ret = tasking_kill(thread, param2);
    return_with_val(ret);
}

void sys_setpgid(trapframe_t* tf)
{
    uint32_t pid = param1;
    uint32_t pgid = param2;

    proc_t* p = tasking_get_proc(pid);
    if (!p) {
        return_with_val(-ESRCH);
    }

    p->pgid = pgid;
    return_with_val(0);
}

void sys_getpgid(trapframe_t* tf)
{
    uint32_t pid = param1;

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

    thread_create_params_t* params = (thread_create_params_t*)param1;
    set_instruction_pointer(thread->tf, params->entry_point);
    uint32_t esp = params->stack_start + params->stack_size;
    set_stack_pointer(thread->tf, esp);
    set_base_pointer(thread->tf, esp);

    return_with_val(thread->tid);
}

void sys_sleep(trapframe_t* tf)
{
    thread_t* p = RUNNING_THREAD;
    time_t time = param1;

    init_sleep_blocker(p, time);

    return_with_val(0);
}

void sys_sched_yield(trapframe_t* tf)
{
    resched();
}

void sys_nice(trapframe_t* tf)
{
    int inc = param1;
    thread_t* thread = RUNNING_THREAD;
    if ((thread->process->prio + inc) < MAX_PRIO || (thread->process->prio + inc) > MIN_PRIO) {
        return_with_val(-1);
    }
    thread->process->prio += inc;
    return_with_val(0);
}