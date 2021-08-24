/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH32_TASKING_DUMP_IMPL_H
#define _KERNEL_PLATFORM_AARCH32_TASKING_DUMP_IMPL_H

#include <libkern/types.h>
#include <tasking/bits/dump.h>
#include <tasking/tasking.h>

int dump_impl(dump_data_t* data);
int dump_kernel_impl(dump_data_t* dump_data, const char* err_desc);
int dump_kernel_impl_from_tf(dump_data_t* dump_data, const char* err_desc, trapframe_t* tf);

#endif // _KERNEL_PLATFORM_AARCH32_TASKING_DUMP_IMPL_H