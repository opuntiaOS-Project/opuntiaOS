/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_MEM_VM_ALLOC_H
#define _KERNEL_MEM_VM_ALLOC_H

#include <libkern/libkern.h>
#include <mem/bits/vm.h>
#include <mem/kmemzone.h>

uintptr_t vm_alloc_pdir_paddr();
uintptr_t vm_alloc_ptable_paddr(ptable_lv_t lv);
uintptr_t vm_alloc_ptables_to_cover_page();
void vm_free_ptables_to_cover_page(uintptr_t addr);
uintptr_t vm_alloc_page_paddr();
void vm_free_page_paddr(uintptr_t addr);

int vm_alloc_mapped_zone(size_t size, size_t alignment, kmemzone_t* zone);
int vm_free_mapped_zone(kmemzone_t zone);

ptable_t* vm_alloc_ptable_lv_top();
void vm_free_ptable_lv_top(ptable_t* pdir);

#endif // _KERNEL_MEM_VM_ALLOC_H
