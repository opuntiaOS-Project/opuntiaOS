/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/bits/errno.h>
#include <libkern/bits/sys/ptrace.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <platform/generic/syscalls/params.h>
#include <syscalls/handlers.h>
#include <tasking/tasking.h>

void sys_ptrace(trapframe_t* tf)
{
    ptrace_request_t request = SYSCALL_VAR1(tf);
    if (request == PTRACE_TRACEME) {
        RUNNING_THREAD->process->is_tracee = true;
        return_with_val(0);
    }

    thread_t* thread = thread_by_pid(SYSCALL_VAR2(tf));
    if (!thread) {
        return_with_val(-ESRCH);
    }

    switch (request) {
    case PTRACE_CONT:
        tasking_signal(thread, SIGCONT);
        break;

    default:
        return_with_val(-EINVAL);
    }

    return_with_val(0);
}