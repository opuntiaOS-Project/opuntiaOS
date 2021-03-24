/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_LIBKERN_KASSERT_H
#define _KERNEL_LIBKERN_KASSERT_H

#include <libkern/c_attrs.h>
#include <libkern/log.h>
#include <libkern/types.h>
#include <platform/generic/system.h>
#include <platform/generic/tasking/trapframe.h>

#define ASSERT(x)                                              \
    if (unlikely(!(x))) {                                      \
        log("kassert at line %d in %s\n", __LINE__, __FILE__); \
        system_stop();                                         \
    }

void kpanic(const char* msg) NORETURN;
void kpanic_tf(const char* err_msg, trapframe_t* tf) NORETURN;

#endif // _KERNEL_LIBKERN_KASSERT_H
