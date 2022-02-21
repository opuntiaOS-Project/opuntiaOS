/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_VM_ADDRESS_SPACE_H
#define _KERNEL_MEM_VM_ADDRESS_SPACE_H

#include <algo/dynamic_array.h>
#include <libkern/libkern.h>
#include <libkern/lock.h>
#include <mem/bits/vm.h>

struct vm_address_space {
    ptable_t* pdir;
    dynamic_array_t zones;
    int count;
    lock_t lock;
};
typedef struct vm_address_space vm_address_space_t;

vm_address_space_t* vm_address_space_alloc();
int vm_address_space_free(vm_address_space_t* old);

#endif // _KERNEL_MEM_VM_ADDRESS_SPACE_H
