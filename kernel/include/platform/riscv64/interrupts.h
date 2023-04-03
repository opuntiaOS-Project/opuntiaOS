/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_RISCV64_INTERRUPTS_H
#define _KERNEL_PLATFORM_RISCV64_INTERRUPTS_H

#include <drivers/irq/irq_api.h>
#include <libkern/mask.h>
#include <libkern/types.h>

void interrupts_setup();
void interrupts_setup_secondary_cpu();

void plic_setup();

#endif /* _KERNEL_PLATFORM_RISCV64_INTERRUPTS_H */