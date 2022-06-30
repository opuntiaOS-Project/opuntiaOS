/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_ARM64_SYSCALLS_PARAMS_H
#define _KERNEL_PLATFORM_ARM64_SYSCALLS_PARAMS_H

#include <platform/arm64/tasking/trapframe.h>

#define SYSCALL_ID(tf) (tf->x[8])
#define SYSCALL_VAR1(tf) (tf->x[0])
#define SYSCALL_VAR2(tf) (tf->x[1])
#define SYSCALL_VAR3(tf) (tf->x[2])
#define SYSCALL_VAR4(tf) (tf->x[3])
#define SYSCALL_VAR5(tf) (tf->x[4])
#define return_val (tf->x[0])
#define return_with_val(val) \
    (return_val = val);      \
    return

#endif // _KERNEL_PLATFORM_ARM64_SYSCALLS_PARAMS_H
