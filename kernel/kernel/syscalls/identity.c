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

void sys_getpid(trapframe_t* tf)
{
    return_with_val(RUNNING_THREAD->tid);
}

void sys_getuid(trapframe_t* tf)
{
    return_with_val(RUNNING_THREAD->process->uid);
}

void sys_setuid(trapframe_t* tf)
{
    uid_t new_uid = param1;
    proc_t* proc = RUNNING_THREAD->process;

    lock_acquire(&proc->lock);

    if (proc->uid != new_uid && proc->euid != new_uid && !proc_is_su(proc)) {
        lock_release(&proc->lock);
        return_with_val(-EPERM);
    }

    proc->uid = new_uid;
    proc->euid = new_uid;
    proc->suid = new_uid;
    lock_release(&proc->lock);
    return_with_val(0);
}

void sys_setgid(trapframe_t* tf)
{
    gid_t new_gid = param1;
    proc_t* proc = RUNNING_THREAD->process;

    lock_acquire(&proc->lock);

    if (proc->gid != new_gid && proc->egid != new_gid && !proc_is_su(proc)) {
        lock_release(&proc->lock);
        return_with_val(-EPERM);
    }

    proc->gid = new_gid;
    proc->egid = new_gid;
    proc->sgid = new_gid;
    lock_release(&proc->lock);
    return_with_val(0);
}

void sys_setreuid(trapframe_t* tf)
{
    proc_t* proc = RUNNING_THREAD->process;
    uid_t new_ruid = param1;
    uid_t new_euid = param2;

    lock_acquire(&proc->lock);

    if (new_ruid == (uid_t)-1) {
        new_ruid = proc->uid;
    }

    if (new_euid == (uid_t)-1) {
        new_euid = proc->euid;
    }

    if (proc->uid != new_euid && proc->euid != new_euid && proc->suid != new_euid) {
        lock_release(&proc->lock);
        return_with_val(-EPERM);
    }

    if (proc->uid != new_ruid && proc->euid != new_ruid && proc->suid != new_ruid) {
        lock_release(&proc->lock);
        return_with_val(-EPERM);
    }

    proc->uid = new_ruid;
    proc->euid = new_euid;
    lock_release(&proc->lock);
    return_with_val(0);
}

void sys_setregid(trapframe_t* tf)
{
    proc_t* proc = RUNNING_THREAD->process;
    gid_t new_rgid = param1;
    gid_t new_egid = param2;

    lock_acquire(&proc->lock);

    if (new_rgid == (gid_t)-1) {
        new_rgid = proc->gid;
    }

    if (new_egid == (gid_t)-1) {
        new_egid = proc->egid;
    }

    if (proc->gid != new_egid && proc->egid != new_egid && proc->sgid != new_egid) {
        lock_release(&proc->lock);
        return_with_val(-EPERM);
    }

    if (proc->gid != new_rgid && proc->egid != new_rgid && proc->sgid != new_rgid) {
        lock_release(&proc->lock);
        return_with_val(-EPERM);
    }

    proc->gid = new_rgid;
    proc->egid = new_egid;

    lock_release(&proc->lock);
    return_with_val(0);
}