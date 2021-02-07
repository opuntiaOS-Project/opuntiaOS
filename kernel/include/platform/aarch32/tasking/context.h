#ifndef __oneOS__AARCH32__CONTEXT_H
#define __oneOS__AARCH32__CONTEXT_H

#include <types.h>

typedef struct {
    uint32_t r[9];
    uint32_t lr;
} __attribute__((packed)) context_t;

static inline uint32_t context_get_instruction_pointer(context_t* ctx)
{
    return ctx->lr;
}

static inline uint32_t context_set_instruction_pointer(context_t* ctx, uint32_t ip)
{
    ctx->lr = ip;
}

#endif // __oneOS__AARCH32__CONTEXT_H