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
#include <libkern/version.h>
#include <mem/vmm.h>
#include <platform/generic/syscalls/params.h>
#include <syscalls/handlers.h>

void sys_uname(trapframe_t* tf)
{
    utsname_t __user* buf = (utsname_t __user*)SYSCALL_VAR1(tf);
    umem_copy_to_user(buf->sysname, OSTYPE, sizeof(OSTYPE));
    umem_copy_to_user(buf->release, OSRELEASE, sizeof(OSRELEASE));
    umem_copy_to_user(buf->version, VERSION_VARIANT, sizeof(VERSION_VARIANT));
    umem_copy_to_user(buf->machine, MACHINE, sizeof(MACHINE));
    return_with_val(0);
}