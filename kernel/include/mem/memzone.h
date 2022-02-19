/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_MEMZONE_H
#define _KERNEL_MEM_MEMZONE_H

#include <algo/dynamic_array.h>
#include <fs/vfs.h>
#include <libkern/types.h>
#include <mem/bits/zone.h>

struct vm_ops;
struct memzone {
    uintptr_t start;
    size_t len;
    uint32_t type;
    uint32_t flags;
    dentry_t* file;
    uintptr_t offset;
    struct vm_ops* ops;
};
typedef struct memzone memzone_t;

struct vm_address_space;
memzone_t* memzone_new(struct vm_address_space* vm_aspace, size_t start, size_t len);
memzone_t* memzone_extend(struct vm_address_space* vm_aspaceroc, size_t start, size_t len);
memzone_t* memzone_new_random(struct vm_address_space* vm_aspace, size_t len);
memzone_t* memzone_new_random_backward(struct vm_address_space* vm_aspace, size_t len);
memzone_t* memzone_find(struct vm_address_space* vm_aspace, size_t addr);
memzone_t* memzone_find_no_proc(dynamic_array_t* zones, size_t addr);
int memzone_free_no_proc(dynamic_array_t*, memzone_t*);
int memzone_free(struct vm_address_space* vm_aspace, memzone_t*);

int memzone_copy(struct vm_address_space* to_vm_aspace, struct vm_address_space* from_vm_aspace);

#endif // _KERNEL_MEM_MEMZONE_H