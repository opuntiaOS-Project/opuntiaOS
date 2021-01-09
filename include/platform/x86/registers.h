/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__X86__REGS_H
#define __oneOS__X86__REGS_H

#include <types.h>

uint32_t read_cr2();
uint32_t read_cr3();
uint32_t read_esp();

#endif /* __oneOS__X86__REGS_H */