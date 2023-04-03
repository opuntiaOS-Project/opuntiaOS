/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_RISCV64_TASKING_TRAPFRAME_H
#define _KERNEL_PLATFORM_RISCV64_TASKING_TRAPFRAME_H

#include <libkern/c_attrs.h>
#include <libkern/log.h>
#include <libkern/types.h>

typedef struct {
    uint64_t ra;
    uint64_t sp;
    uint64_t gp;
    uint64_t tp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t s0;
    uint64_t s1;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
    uint64_t sstatus;
    uint64_t epc;
} PACKED trapframe_t;

static inline uintptr_t get_stack_pointer(trapframe_t* tf)
{
    return tf->sp;
}

static inline void set_stack_pointer(trapframe_t* tf, uintptr_t sp)
{
    tf->sp = sp;
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
    return tf->epc;
}

static inline void set_instruction_pointer(trapframe_t* tf, uintptr_t ip)
{
    tf->epc = ip;
}

static inline uintptr_t get_syscall_result(trapframe_t* tf)
{
    return tf->a0;
}

static inline void set_syscall_result(trapframe_t* tf, uintptr_t val)
{
    tf->a0 = val;
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

#define SSTATUS_VM (1L << 18) // VM
#define SSTATUS_SPP (1L << 8) // Previous mode, 1=Supervisor, 0=User
#define SSTATUS_SPIE (1L << 5) // Supervisor Previous Interrupt Enable
#define SSTATUS_UPIE (1L << 4) // User Previous Interrupt Enable
#define SSTATUS_SIE (1L << 1) // Supervisor Interrupt Enable
#define SSTATUS_UIE (1L << 0) // User Interrupt Enable

static inline void tf_setup_as_user_thread(trapframe_t* tf)
{
    tf->sstatus = SSTATUS_VM | SSTATUS_UPIE | SSTATUS_SPIE;
}

static inline void tf_setup_as_kernel_thread(trapframe_t* tf)
{
    tf->sstatus = SSTATUS_VM | SSTATUS_SPP | SSTATUS_SPIE;
}

static void dump_tf(trapframe_t* tf)
{
}

#endif // _KERNEL_PLATFORM_RISCV64_TASKING_TRAPFRAME_H