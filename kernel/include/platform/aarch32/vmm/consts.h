/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef __oneOS__AARCH32__VMM__CONSTS_H
#define __oneOS__AARCH32__VMM__CONSTS_H

#define VMM_PTE_COUNT (256)
#define VMM_PDE_COUNT (4096)
#define VMM_PAGE_SIZE (4096)

#define VMM_OFFSET_IN_DIRECTORY(a) (((a) >> 20) & 0xfff)
#define VMM_OFFSET_IN_TABLE(a) (((a) >> 12) & 0xff)
#define VMM_OFFSET_IN_PAGE(a) ((a)&0xfff)
#define TABLE_START(vaddr) ((vaddr >> 20) << 20)
#define PAGE_START(vaddr) ((vaddr >> 12) << 12)
#define FRAME(addr) (addr / VMM_PAGE_SIZE)

#define VMM_USER_TABLES_START 0
#define VMM_KERNEL_TABLES_START 3072

#endif //__oneOS__AARCH32__VMM__TABLE_DESC_H
