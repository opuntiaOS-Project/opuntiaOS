#ifndef _KERNEL_PLATFORM_AARCH32_TASKING_TRAPFRAME_H
#define _KERNEL_PLATFORM_AARCH32_TASKING_TRAPFRAME_H

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

static inline uint32_t get_stack_pointer(trapframe_t* tf)
{
    return tf->user_sp;
}

static inline void set_stack_pointer(trapframe_t* tf, uint32_t sp)
{
    tf->user_sp = sp;
}

static inline uint32_t get_base_pointer(trapframe_t* tf)
{
    return 0;
}

static inline void set_base_pointer(trapframe_t* tf, uint32_t bp)
{
}

static inline uint32_t get_instruction_pointer(trapframe_t* tf)
{
    return tf->user_ip;
}

static inline void set_instruction_pointer(trapframe_t* tf, uint32_t ip)
{
    tf->user_ip = ip;
}

static inline uint32_t get_syscall_result(trapframe_t* tf)
{
    return tf->r[0];
}

static inline void set_syscall_result(trapframe_t* tf, uint32_t val)
{
    tf->r[0] = val;
}

/**
 * STACK FUNCTIONS
 */

static inline void tf_push_to_stack(trapframe_t* tf, uint32_t val)
{
    tf->user_sp -= 4;
    *((uint32_t*)tf->user_sp) = val;
}

static inline uint32_t tf_pop_to_stack(trapframe_t* tf)
{
    uint32_t val = *((uint32_t*)tf->user_sp);
    tf->user_sp += 4;
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

#endif // _KERNEL_PLATFORM_AARCH32_TASKING_TRAPFRAME_H