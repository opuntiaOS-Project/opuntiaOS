/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_VMM_PF_TYPES_H
#define _KERNEL_PLATFORM_X86_VMM_PF_TYPES_H

#include <libkern/types.h>

static inline int _vmm_is_caused_by_user(uint32_t info)
{
    return ((info & 0b100) == 0b100);
}

static inline int _vmm_is_caused_by_kernel(uint32_t info)
{
    return !_vmm_is_caused_by_user(info);
}

static inline int _vmm_is_caused_writing(uint32_t info)
{
    return ((info & 0b010) == 0b010);
}

static inline int _vmm_is_caused_reading(uint32_t info)
{
    return ((info & 0b010) == 0);
}

static inline int _vmm_is_table_not_present(uint32_t info)
{
    return ((info & 0b001) == 0);
}

static inline int _vmm_is_page_not_present(uint32_t info)
{
    return ((info & 0b001) == 0);
}

static inline int _vmm_is_table_permission_fault(uint32_t info)
{
    return !_vmm_is_page_not_present(info);
}

static inline int _vmm_is_page_permission_fault(uint32_t info)
{
    return !_vmm_is_page_not_present(info);
}

#endif //_KERNEL_PLATFORM_X86_VMM_PF_TYPES_H
