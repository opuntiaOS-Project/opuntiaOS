/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_TASKING_DUMP_H
#define _KERNEL_TASKING_DUMP_H

#include <platform/generic/tasking/trapframe.h>
#include <tasking/proc.h>

void dump_and_kill(proc_t* p);
int dump_prepare_kernel_data();
int dump_kernel(const char* err);
int dump_kernel_from_tf(const char* err, trapframe_t* tf);

#endif // _KERNEL_TASKING_DUMP_H