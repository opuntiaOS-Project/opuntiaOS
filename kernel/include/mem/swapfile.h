/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_SWAPFILE_H
#define _KERNEL_MEM_SWAPFILE_H

#include <libkern/libkern.h>
#include <mem/bits/mmu.h>
#include <mem/bits/swap.h>
#include <platform/generic/vmm/consts.h>

int swapfile_init();
int swapfile_new_ref(int id);
int swapfile_rem_ref(int id);
int swapfile_load(uintptr_t vaddr, int id);
int swapfile_store(uintptr_t vaddr);

#endif // _KERNEL_MEM_SWAPFILE_H
