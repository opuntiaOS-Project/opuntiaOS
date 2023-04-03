/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_RISCV64_SYSCALLS_PARAMS_H
#define _KERNEL_PLATFORM_RISCV64_SYSCALLS_PARAMS_H

#include <platform/riscv64/tasking/trapframe.h>

#define SYSCALL_ID(tf) (tf->a7)
#define SYSCALL_VAR1(tf) (tf->a0)
#define SYSCALL_VAR2(tf) (tf->a1)
#define SYSCALL_VAR3(tf) (tf->a2)
#define SYSCALL_VAR4(tf) (tf->a3)
#define SYSCALL_VAR5(tf) (tf->a4)
#define return_val (tf->a0)
#define return_with_val(val) \
    (return_val = val);      \
    return

#endif // _KERNEL_PLATFORM_RISCV64_SYSCALLS_PARAMS_H
