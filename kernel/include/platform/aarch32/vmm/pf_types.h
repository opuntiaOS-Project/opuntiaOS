/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 * generic
 */

#ifndef _KERNEL_PLATFORM_AARCH32_VMM_PF_TYPES_H
#define _KERNEL_PLATFORM_AARCH32_VMM_PF_TYPES_H

#include <libkern/types.h>

static inline int _vmm_is_caused_by_user(uint32_t info) 
{
    return (((info >> 31) & 0x1) == 0); // Check for pl0, see aarch32/interrupt_handlers.c
}

static inline int _vmm_is_caused_by_kernel(uint32_t info) 
{
    return !_vmm_is_caused_by_user(info);
}

static inline int _vmm_is_caused_writing(uint32_t info) 
{
    return ((info >> 11) & 0x1);
}

static inline int _vmm_is_caused_reading(uint32_t info) 
{
    return (((info >> 11) & 0x1) == 0);
}

static inline int _vmm_is_table_not_present(uint32_t info) 
{
    return ((info & 0b1111) == 0b0101);
}

static inline int _vmm_is_page_not_present(uint32_t info) 
{
    return ((info & 0b1111) == 0b0111);
}

static inline int _vmm_is_table_permission_fault(uint32_t info) 
{
    return ((info & 0b1111) == 0b1101);
}

static inline int _vmm_is_page_permission_fault(uint32_t info) 
{
    return ((info & 0b1111) == 0b1111);
}

#endif //_KERNEL_PLATFORM_AARCH32_VMM_PF_TYPES_H
