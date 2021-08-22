/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_IRQ_HANDLER_H
#define _KERNEL_PLATFORM_X86_IRQ_HANDLER_H

#include <drivers/x86/display.h>
#include <platform/x86/idt.h>

void irq_handler(trapframe_t* tf);
void irq_empty_handler();

#endif