/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_ISR_HANDLER_H
#define _KERNEL_PLATFORM_X86_ISR_HANDLER_H

#include <libkern/types.h>
#include <platform/x86/idt.h>

void isr_handler(trapframe_t* tf);

#endif
