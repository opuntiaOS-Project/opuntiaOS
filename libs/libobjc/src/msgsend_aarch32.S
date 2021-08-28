.extern objc_msg_lookup
.global objc_msgSend

objc_msgSend:
    push    {r0-r3}
    push    {lr}
    bl      objc_msg_lookup
    pop     {lr}
    mov     r12, r0 // imp pointer
    pop     {r0-r3}
    bx      r12
