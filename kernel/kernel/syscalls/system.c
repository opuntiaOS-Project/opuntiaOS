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
    utsname_t* buf = (utsname_t*)SYSCALL_VAR1(tf);
    vmm_copy_to_user(buf->sysname, OSTYPE, sizeof(OSTYPE));
    vmm_copy_to_user(buf->release, OSRELEASE, sizeof(OSRELEASE));
    vmm_copy_to_user(buf->version, VERSION_VARIANT, sizeof(VERSION_VARIANT));
    vmm_copy_to_user(buf->machine, MACHINE, sizeof(MACHINE));
    return_with_val(0);
}