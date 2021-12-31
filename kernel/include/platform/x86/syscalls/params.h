/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_SYSCALLS_PARAMS_H
#define _KERNEL_PLATFORM_X86_SYSCALLS_PARAMS_H

#include <platform/x86/tasking/trapframe.h>

#define SYSCALL_HANDLER_NO 0x80

#define SYSCALL_ID(tf) (tf->eax)
#define SYSCALL_VAR1(tf) (tf->ebx)
#define SYSCALL_VAR2(tf) (tf->ecx)
#define SYSCALL_VAR3(tf) (tf->edx)
#define SYSCALL_VAR4(tf) (tf->esi)
#define SYSCALL_VAR5(tf) (tf->edi)
#define return_val (tf->eax)
#define return_with_val(val) \
    (return_val = val);      \
    return

#endif // _KERNEL_PLATFORM_X86_SYSCALLS_PARAMS_H
