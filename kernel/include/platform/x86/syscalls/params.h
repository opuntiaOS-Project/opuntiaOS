/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_SYSCALLS_PARAMS_H
#define _KERNEL_PLATFORM_X86_SYSCALLS_PARAMS_H

#include <platform/x86/tasking/trapframe.h>

#define SYSCALL_HANDLER_NO 0x80

#define sys_id (tf->eax)
#define param1 (tf->ebx)
#define param2 (tf->ecx)
#define param3 (tf->edx)
#define param4 (tf->esi)
#define param5 (tf->edi)
#define return_val (tf->eax)
#define return_with_val(val) \
    (return_val = val);      \
    return

#endif // _KERNEL_PLATFORM_X86_SYSCALLS_PARAMS_H
