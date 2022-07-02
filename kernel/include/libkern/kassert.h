/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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

void assert_handler(const char* cond, const char* func, const char* file, int line) NORETURN;
#define ASSERT(x)                                         \
    if (unlikely(!(x))) {                                 \
        assert_handler(#x, __func__, __FILE__, __LINE__); \
    }

#ifdef DEBUG_KERNEL
#define DEBUG_ASSERT(x)                                   \
    if (unlikely(!(x))) {                                 \
        assert_handler(#x, __func__, __FILE__, __LINE__); \
    }
#else
#undef DEBUG_ASSERT
#define DEBUG_ASSERT(x)
#endif

#define __IMPL_SASSERT_PASTE(a, b) a##b
#define __IMPL_SASSERT_LINE(predicate, line, file) \
    typedef char __IMPL_SASSERT_PASTE(assertion_failed_##file##_, line)[2 * !!(predicate)-1]

#define STATIC_ASSERT(predicate, file) __IMPL_SASSERT_LINE(predicate, __LINE__, file)

void kpanic(const char* msg) NORETURN;
void kpanic_tf(const char* err_msg, trapframe_t* tf) NORETURN;

#endif // _KERNEL_LIBKERN_KASSERT_H
