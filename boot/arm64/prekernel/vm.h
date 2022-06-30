/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_VM_H
#define _BOOT_VM_H

#include <libboot/abi/memory.h>
#include <libboot/abi/rawimage.h>
#include <libboot/types.h>

#define VMM_LV0_ENTITY_COUNT (512)
#define VMM_LV1_ENTITY_COUNT (512)
#define VMM_LV2_ENTITY_COUNT (512)
#define VMM_LV3_ENTITY_COUNT (512)

#define PTABLE_LV_TOP (2)
#define PTABLE_LV0_VADDR_OFFSET (12)
#define PTABLE_LV1_VADDR_OFFSET (21)
#define PTABLE_LV2_VADDR_OFFSET (30)
#define PTABLE_LV3_VADDR_OFFSET (39)

#define VM_VADDR_OFFSET_AT_LEVEL(vaddr, off, ent) ((vaddr >> off) % ent)

static size_t page_size() { return 0x1000; }

void vm_setup(uintptr_t base, boot_args_t* args, rawimage_header_t* riheader);

#endif // _BOOT_VM_H