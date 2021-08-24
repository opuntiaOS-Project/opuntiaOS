/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_TASKING_SWITCHVM_H
#define _KERNEL_PLATFORM_X86_TASKING_SWITCHVM_H

#include <libkern/types.h>
#include <tasking/tasking.h>

void switchuvm(thread_t* thread);

#endif