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

// TODO: Make it not depandent on proc.
struct proc;
memzone_t* memzone_new(struct proc* p, size_t start, size_t len);
memzone_t* memzone_extend(struct proc* proc, size_t start, size_t len);
memzone_t* memzone_new_random(struct proc* p, size_t len);
memzone_t* memzone_new_random_backward(struct proc* p, size_t len);
memzone_t* memzone_find(struct proc* p, size_t addr);
memzone_t* memzone_find_no_proc(dynamic_array_t* zones, size_t addr);
int memzone_free_no_proc(dynamic_array_t*, memzone_t*);
int memzone_free(struct proc*, memzone_t*);

#endif // _KERNEL_MEM_MEMZONE_H