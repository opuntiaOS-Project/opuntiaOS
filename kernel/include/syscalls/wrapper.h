/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_SYSCALLS_WRAPPER_H
#define _KERNEL_SYSCALLS_WRAPPER_H

#define SYSCALL_INIT_VAR(p, val) p = (typeof(p))val
#define SYSCALL_INIT_VARS1(a) SYSCALL_INIT_VAR(a, SYSCALL_VAR1(tf))
#define SYSCALL_INIT_VARS2(a, b) SYSCALL_INIT_VAR(a, SYSCALL_VAR1(tf)), SYSCALL_INIT_VAR(b, SYSCALL_VAR2(tf))
#define SYSCALL_INIT_VARS3(a, b, c) SYSCALL_INIT_VAR(a, SYSCALL_VAR1(tf)), SYSCALL_INIT_VAR(b, SYSCALL_VAR2(tf)), SYSCALL_INIT_VAR(c, SYSCALL_VAR3(tf))
#define SYSCALL_INIT_VARS4(a, b, c, d) SYSCALL_INIT_VAR(a, SYSCALL_VAR1(tf)), SYSCALL_INIT_VAR(b, SYSCALL_VAR2(tf)), SYSCALL_INIT_VAR(c, SYSCALL_VAR3(tf)), SYSCALL_INIT_VAR(d, SYSCALL_VAR4(tf))
#define SYSCALL_INIT_VARS5(a, b, c, d, e) SYSCALL_INIT_VAR(a, SYSCALL_VAR1(tf)), SYSCALL_INIT_VAR(b, SYSCALL_VAR2(tf)), SYSCALL_INIT_VAR(c, SYSCALL_VAR3(tf)), SYSCALL_INIT_VAR(d, SYSCALL_VAR4(tf)), SYSCALL_INIT_VAR(e, SYSCALL_VAR5(tf))

#endif // _KERNEL_SYSCALLS_WRAPPER_H
