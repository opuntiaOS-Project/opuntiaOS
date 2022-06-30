.global switch_contexts
.global switch_to_context

// void switch_contexts(context_t** old, context_t* new)
switch_contexts:
    sub sp, sp, #0xb8
    
    stp x8, x9, [sp,#0x0]
    stp x10, x11, [sp,#0x10]
    stp x12, x13, [sp,#0x20]
    stp x14, x15, [sp,#0x30]
    stp x16, x17, [sp,#0x40]
    stp x18, x19, [sp,#0x50]
    stp x20, x21, [sp,#0x60]
    stp x22, x23, [sp,#0x70]
    stp x24, x25, [sp,#0x80]
    stp x26, x27, [sp,#0x90]
    stp x28, x29, [sp,#0xa0]
    mov x2, lr
    str x2, [sp,#0xb0]
    
    mov x2, sp
    str x2, [x0]
    mov sp, x1
    
    ldp x8,x9,[sp,#0x0]
    ldp x10,x11,[sp,#0x10]
    ldp x12,x13,[sp,#0x20]
    ldp x14,x15,[sp,#0x30]
    ldp x16,x17,[sp,#0x40]
    ldp x18,x19,[sp,#0x50]
    ldp x20,x21,[sp,#0x60]
    ldp x22,x23,[sp,#0x70]
    ldp x24,x25,[sp,#0x80]
    ldp x26,x27,[sp,#0x90]
    ldp x28,x29,[sp,#0xa0]
    ldr x2,[sp,#0xb0]
    mov lr, x2

    add sp, sp, #0xb8
    ret

// void switch_to_context(context_t* new)
switch_to_context:
    mov sp, x0
    ldp x8,x9,[sp,#0x0]
    ldp x10,x11,[sp,#0x10]
    ldp x12,x13,[sp,#0x20]
    ldp x14,x15,[sp,#0x30]
    ldp x16,x17,[sp,#0x40]
    ldp x18,x19,[sp,#0x50]
    ldp x20,x21,[sp,#0x60]
    ldp x22,x23,[sp,#0x70]
    ldp x24,x25,[sp,#0x80]
    ldp x26,x27,[sp,#0x90]
    ldp x28,x29,[sp,#0xa0]
    ldr x0,[sp,#0xb0]
    mov lr, x0

    add sp, sp, #0xb8
    ret
