/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_PORT_H
#define _KERNEL_PLATFORM_X86_PORT_H

#include <libkern/types.h>

uint8_t port_read8(uint16_t port);
void port_write8(uint16_t port, uint8_t data);
uint16_t port_read16(uint16_t port);
void port_write16(uint16_t port, uint16_t data);
uint32_t port_read32(uint16_t port);
void port_write32(uint16_t port, uint32_t data);
uint8_t port_read8(uint16_t port);
void port_write8(uint16_t port, uint8_t data);
uint16_t port_read16(uint16_t port);
void port_write16(uint16_t port, uint16_t data);
uint32_t port_read32(uint16_t port);
void port_write32(uint16_t port, uint32_t data);
void port_wait_io();

#endif