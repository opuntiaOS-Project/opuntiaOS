/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH64_TASKING_TRAPFRAME_H
#define _KERNEL_PLATFORM_AARCH64_TASKING_TRAPFRAME_H

#include <libkern/c_attrs.h>
#include <libkern/log.h>
#include <libkern/types.h>

typedef struct {
    uint64_t x[31];
    uint64_t esr;
    uint64_t elr;
    uint64_t far;
    uint64_t spsr;
    uint64_t sp;
} PACKED trapframe_t;

static inline uintptr_t get_stack_pointer(trapframe_t* tf)
{
    return tf->sp;
}

static inline void set_stack_pointer(trapframe_t* tf, uintptr_t sp)
{
    tf->sp = sp;
}

static inline uintptr_t get_base_pointer(trapframe_t* tf)
{
    return 0;
}

static inline void set_base_pointer(trapframe_t* tf, uintptr_t bp)
{
}

static inline uintptr_t get_instruction_pointer(trapframe_t* tf)
{
    return tf->elr;
}

static inline void set_instruction_pointer(trapframe_t* tf, uintptr_t ip)
{
    tf->elr = ip;
}

static inline uintptr_t get_syscall_result(trapframe_t* tf)
{
    return tf->x[0];
}

static inline void set_syscall_result(trapframe_t* tf, uintptr_t val)
{
    tf->x[0] = val;
}

/**
 * STACK FUNCTIONS
 */

static inline void tf_push_to_stack(trapframe_t* tf, uintptr_t val)
{
    tf->sp -= sizeof(uintptr_t);
    *((uintptr_t*)tf->sp) = val;
}

static inline uintptr_t tf_pop_to_stack(trapframe_t* tf)
{
    uintptr_t val = *((uintptr_t*)tf->sp);
    tf->sp += sizeof(uintptr_t);
    return val;
}

static inline void tf_move_stack_pointer(trapframe_t* tf, int32_t val)
{
    tf->sp += val;
}

static inline void tf_setup_as_user_thread(trapframe_t* tf)
{
    tf->spsr = 0x0;
}

static inline void tf_setup_as_kernel_thread(trapframe_t* tf)
{
    tf->spsr = 0x4;
}

static void dump_tf(trapframe_t* tf)
{
    // for (int i = 0; i < 31; i++) {
    //     log("x[%d]: %zx", i, tf->x[i]);
    // }

    // log("tf: %p", tf);
    // log("sp: %zx", tf->sp);
    // log("ip: %zx", tf->elr);
    // log("fl: %zx", tf->spsr);
    // log("far: %zx", tf->far);
    // log("esr: %zx", tf->esr);
}

#endif // _KERNEL_PLATFORM_AARCH64_TASKING_TRAPFRAME_H