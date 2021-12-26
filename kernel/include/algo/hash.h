/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_ALGO_HASH_H
#define _KERNEL_ALGO_HASH_H

#include <libkern/types.h>

#define hashint(hfunc, val) (hfunc((uint8_t*)&val, sizeof(val)))

uint32_t hash_crc32(uint8_t* data, size_t len);
uint32_t hashstr_crc32(char* data);

#endif // _KERNEL_ALGO_HASH_H