/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_ARM32_TASKING_TRAPFRAME_H
#define _KERNEL_PLATFORM_ARM32_TASKING_TRAPFRAME_H

#include <libkern/c_attrs.h>
#include <libkern/log.h>
#include <libkern/types.h>

#define CPSR_M_USR 0x10U
#define CPSR_M_FIQ 0x11U
#define CPSR_M_IRQ 0x12U
#define CPSR_M_SVC 0x13U
#define CPSR_M_MON 0x16U
#define CPSR_M_ABT 0x17U
#define CPSR_M_HYP 0x1AU
#define CPSR_M_UND 0x1BU
#define CPSR_M_SYS 0x1FU

typedef struct {
    uint32_t user_flags;
    uint32_t user_sp;
    uint32_t user_lr;
    uint32_t r[13];
    uint32_t user_ip;
} PACKED trapframe_t;

static inline uintptr_t get_stack_pointer(trapframe_t* tf)
{
    return tf->user_sp;
}

static inline void set_stack_pointer(trapframe_t* tf, uintptr_t sp)
{
    tf->user_sp = sp;
}

static inline uintptr_t get_frame_pointer(trapframe_t* tf)
{
    return 0;
}

static inline void set_frame_pointer(trapframe_t* tf, uintptr_t bp)
{
}

static inline uintptr_t get_instruction_pointer(trapframe_t* tf)
{
    return tf->user_ip;
}

static inline void set_instruction_pointer(trapframe_t* tf, uintptr_t ip)
{
    tf->user_ip = ip;
}

static inline uint32_t get_syscall_result(trapframe_t* tf)
{
    return tf->r[0];
}

static inline void set_syscall_result(trapframe_t* tf, uintptr_t val)
{
    tf->r[0] = val;
}

/**
 * STACK FUNCTIONS
 */

static inline void tf_push_to_stack(trapframe_t* tf, uintptr_t val)
{
    tf->user_sp -= sizeof(uintptr_t);
    *((uintptr_t*)tf->user_sp) = val;
}

static inline uintptr_t tf_pop_to_stack(trapframe_t* tf)
{
    uintptr_t val = *((uintptr_t*)tf->user_sp);
    tf->user_sp += sizeof(uintptr_t);
    return val;
}

static inline void tf_move_stack_pointer(trapframe_t* tf, int32_t val)
{
    tf->user_sp += val;
}

static inline void tf_setup_as_user_thread(trapframe_t* tf)
{
    tf->user_flags = 0x60000100 | CPSR_M_USR;
}

static inline void tf_setup_as_kernel_thread(trapframe_t* tf)
{
    tf->user_flags = 0x60000100 | CPSR_M_SYS;
}

static void dump_tf(trapframe_t* tf)
{
    for (int i = 0; i < 13; i++) {
        log("r[%d]: %x", i, tf->r[i]);
    }
    log("sp: %x", tf->user_sp);
    log("ip: %x", tf->user_ip);
    log("fl: %x", tf->user_flags);
}

#endif // _KERNEL_PLATFORM_ARM32_TASKING_TRAPFRAME_H