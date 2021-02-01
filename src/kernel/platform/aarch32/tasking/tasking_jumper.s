.global _tasking_jumper
.global switch_to_context
.extern system_enable_interrupts

// void _tasking_jumper()
_tasking_jumper:
    bl      system_enable_interrupts
    mov     r0, sp // saving current sp to bring it to the new mode
    cps	    #0x13
    mov     sp, r0
    pop     {pc}
