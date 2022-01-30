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

ptable_t* vm_pspace_get_nth_active_ptable(size_t n);
ptable_t* vm_pspace_get_vaddr_of_active_ptable(uintptr_t vaddr);
page_desc_t* vm_pspace_get_page_desc(uintptr_t vaddr);

void vm_pspace_init();
void vm_pspace_gen(pdirectory_t* pdir);
void vm_pspace_free(pdirectory_t* pdir);
int vm_pspace_on_ptable_mapped(uintptr_t vaddr, uintptr_t ptable_paddr, ptable_lv_t level);

#endif // _KERNEL_MEM_VM_PSPACE_H
