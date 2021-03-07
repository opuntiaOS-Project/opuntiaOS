#ifndef _KERNEL_PLATFORM_X86_TASKING_CONTEXT_H
#define _KERNEL_PLATFORM_X86_TASKING_CONTEXT_H

#include <libkern/types.h>

typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
} __attribute__((packed)) context_t;

static inline uint32_t context_get_instruction_pointer(context_t* ctx)
{
    return ctx->eip;
}

static inline void context_set_instruction_pointer(context_t* ctx, uint32_t ip)
{
    ctx->eip = ip;
}

#endif // _KERNEL_PLATFORM_X86_TASKING_CONTEXT_H