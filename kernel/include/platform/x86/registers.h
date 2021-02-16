/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__X86__REGS_H
#define __oneOS__X86__REGS_H

#include <types.h>

uint32_t read_cr2();
uint32_t read_cr3();
uint32_t read_esp();

#endif /* __oneOS__X86__REGS_H */