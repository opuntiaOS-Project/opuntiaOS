/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_ARM32_SYSCALLS_PARAMS_H
#define _KERNEL_PLATFORM_ARM32_SYSCALLS_PARAMS_H

#include <platform/arm32/tasking/trapframe.h>

#define SYSCALL_ID(tf) (tf->r[7])
#define SYSCALL_VAR1(tf) (tf->r[0])
#define SYSCALL_VAR2(tf) (tf->r[1])
#define SYSCALL_VAR3(tf) (tf->r[2])
#define SYSCALL_VAR4(tf) (tf->r[3])
#define SYSCALL_VAR5(tf) (tf->r[4])
#define return_val (tf->r[0])
#define return_with_val(val) \
    (return_val = val);      \
    return

#endif // _KERNEL_PLATFORM_ARM32_SYSCALLS_PARAMS_H
