/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_ARM64_TASKING_SIGNAL_IMPL_H
#define _KERNEL_PLATFORM_ARM64_TASKING_SIGNAL_IMPL_H

#include <libkern/types.h>
struct thread;

int signal_impl_prepare_stack(struct thread* thread, int signo, uintptr_t old_sp, uintptr_t magic);
int signal_impl_restore_stack(struct thread* thread, uintptr_t* old_sp, uintptr_t* magic);

#endif // _KERNEL_PLATFORM_ARM64_TASKING_SIGNAL_IMPL_H