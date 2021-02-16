/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__X86__VMM__CONSTS_H
#define __oneOS__X86__VMM__CONSTS_H

#define VMM_PTE_COUNT (1024)
#define VMM_PDE_COUNT (1024)
#define VMM_PAGE_SIZE (4096)

#define VMM_OFFSET_IN_DIRECTORY(a) (((a) >> 22) & 0x3ff)
#define VMM_OFFSET_IN_TABLE(a) (((a) >> 12) & 0x3ff)
#define VMM_OFFSET_IN_PAGE(a) ((a)&0xfff)
#define TABLE_START(vaddr) ((vaddr >> 22) << 22)
#define PAGE_START(vaddr) ((vaddr >> 12) << 12)
#define FRAME(addr) (addr / VMM_PAGE_SIZE)

#define VMM_USER_TABLES_START 0
#define VMM_KERNEL_TABLES_START 768

#endif //__oneOS__X86__VMM__CONSTS_H
