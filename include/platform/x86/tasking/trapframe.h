#ifndef __oneOS__INTERRUPTS__TRAPFRAME_H
#define __oneOS__INTERRUPTS__TRAPFRAME_H

#include <types.h>

typedef struct {
    // registers as pushed by pusha
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t oesp;      // useless & ignored
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    // rest of trap frame
    uint16_t gs;
    uint16_t padding1;
    uint16_t fs;
    uint16_t padding2;
    uint16_t es;
    uint16_t padding3;
    uint16_t ds;
    uint16_t padding4;
    uint32_t int_no;

    // below here defined by x86 hardware
    uint32_t err;
    uint32_t eip;
    uint16_t cs;
    uint16_t padding5;
    uint32_t eflags;

    // below here only when crossing rings, such as from user to kernel
    uint32_t esp;
    uint16_t ss;
    uint16_t padding6;
} __attribute__((packed)) trapframe_t;

static inline uint32_t get_stack_pointer(trapframe_t* tf)
{
    return tf->esp;
}

static inline uint32_t set_stack_pointer(trapframe_t* tf, uint32_t sp)
{
    tf->esp = sp;
}

static inline uint32_t get_instruction_pointer(trapframe_t* tf)
{
    return tf->eip;
}

static inline uint32_t set_instruction_pointer(trapframe_t* tf, uint32_t ip)
{
    tf->eip = ip;
}

static inline uint32_t get_syscall_result(trapframe_t* tf)
{
    return tf->eax;
}

static inline uint32_t set_syscall_result(trapframe_t* tf, uint32_t val)
{
    tf->eax = val;
}

#endif