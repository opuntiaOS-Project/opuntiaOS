/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_VM_PSPACE_H
#define _KERNEL_MEM_VM_PSPACE_H

#include <libkern/libkern.h>
#include <mem/bits/vm.h>
#include <mem/boot.h>

struct vm_address_space;

ptable_t* vm_pspace_get_nth_active_ptable(size_t n, ptable_lv_t lv);
ptable_t* vm_pspace_get_vaddr_of_active_ptable(uintptr_t vaddr, ptable_lv_t lv);

void vm_pspace_init(boot_args_t* args);
void vm_pspace_gen(ptable_t* pdir);
void vm_pspace_free(ptable_t* pdir);
int vm_pspace_on_ptable_mapped(uintptr_t vaddr, uintptr_t ptable_paddr, ptable_lv_t lv);

ptable_t* vm_get_table(uintptr_t vaddr, ptable_lv_t lv);
ptable_entity_t* vm_get_entity(uintptr_t vaddr, ptable_lv_t lv);
int vm_pspace_free_address_space_locked(struct vm_address_space* vm_aspace);

static inline ptable_entity_t* vm_lookup(ptable_t* table, ptable_lv_t lv, uintptr_t vaddr)
{
    return &table->entities[VM_VADDR_OFFSET_AT_LEVEL(vaddr, lv)];
}

#endif // _KERNEL_MEM_VM_PSPACE_H
