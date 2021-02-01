#ifndef __oneOS__X86__CONTEXT_H
#define __oneOS__X86__CONTEXT_H

#include <types.h>

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

static inline uint32_t context_set_instruction_pointer(context_t* ctx, uint32_t ip)
{
    ctx->eip = ip;
}

#endif // __oneOS__X86__CONTEXT_H