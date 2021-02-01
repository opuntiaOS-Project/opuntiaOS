.global switch_contexts
.global switch_to_context

// void switch_contexts(context_t** old, context_t* new)
switch_contexts:
    stmfd   sp!, {r4-r12,lr}
    str     sp, [r0]
    mov     sp, r1
    ldmfd   sp!, {r4-r12,lr}
    bx lr

// void switch_to_context(context_t* new)
switch_to_context:
    mov     sp, r0
    ldmfd   sp!, {r4-r12,lr}
    bx lr
