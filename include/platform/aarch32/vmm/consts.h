/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
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
