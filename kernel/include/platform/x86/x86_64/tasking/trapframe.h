/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_X86_X86_64_TASKING_TRAPFRAME_H
#define _KERNEL_PLATFORM_X86_X86_64_TASKING_TRAPFRAME_H

#include <libkern/c_attrs.h>
#include <libkern/types.h>
#include <platform/x86/gdt.h>
#include <platform/x86/tasking/tss.h>

struct PACKED trapframe {
    // all registers
    uint64_t r15;
    uint64_t r14;
    uint64_t r13;
    uint64_t r12;
    uint64_t r11;
    uint64_t r10;
    uint64_t r9;
    uint64_t r8;
    uint64_t rbp;
    uint64_t rdi;
    uint64_t rsi;
    uint64_t rdx;
    uint64_t rcx;
    uint64_t rbx;
    uint64_t rax;

    // rest of trap frame
    uint64_t gs;
    uint64_t fs;
    uint64_t trapno;

    // below here defined by x86 hardware
    uint32_t err;
    uint64_t rip;
    uint64_t cs;

    // below here only when crossing rings, such as from user to kernel
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};
typedef struct trapframe trapframe_t;

static inline uintptr_t get_stack_pointer(trapframe_t* tf)
{
    return tf->rsp;
}

static inline void set_stack_pointer(trapframe_t* tf, uintptr_t sp)
{
    tf->rsp = sp;
}

static inline uintptr_t get_frame_pointer(trapframe_t* tf)
{
    return tf->rbp;
}

static inline void set_frame_pointer(trapframe_t* tf, uintptr_t bp)
{
    tf->rbp = bp;
}

static inline uintptr_t get_instruction_pointer(trapframe_t* tf)
{
    return tf->rip;
}

static inline void set_instruction_pointer(trapframe_t* tf, uintptr_t ip)
{
    tf->rip = ip;
}

static inline uintptr_t get_syscall_result(trapframe_t* tf)
{
    return tf->rax;
}

static inline void set_syscall_result(trapframe_t* tf, uintptr_t val)
{
    tf->rax = val;
}

/**
 * STACK FUNCTIONS
 */

static inline void tf_push_to_stack(trapframe_t* tf, uintptr_t val)
{
    tf->rsp -= sizeof(uintptr_t);
    *((uintptr_t*)tf->rsp) = val;
}

static inline uint32_t tf_pop_to_stack(trapframe_t* tf)
{
    uintptr_t val = *((uintptr_t*)tf->rsp);
    tf->rsp += sizeof(uintptr_t);
    return val;
}

static inline void tf_move_stack_pointer(trapframe_t* tf, int32_t val)
{
    tf->rsp += val;
}

static inline void tf_setup_as_user_thread(trapframe_t* tf)
{
    tf->cs = (GDT_SEG_UCODE << 3) | DPL_USER;
    tf->ss = (GDT_SEG_UDATA << 3) | DPL_USER;
    tf->rflags = FL_IF;
}

static inline void tf_setup_as_kernel_thread(trapframe_t* tf)
{
    tf->cs = (GDT_SEG_KCODE << 3);
    tf->ss = (GDT_SEG_KDATA << 3);
    tf->rflags = FL_IF;
}

#endif // _KERNEL_PLATFORM_X86_X86_64_TASKING_TRAPFRAME_H