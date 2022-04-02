.global _tasking_jumper
.global switch_to_context
.extern system_enable_interrupts
.extern _asm_cpu_enter_user_space

// void _tasking_jumper()
_tasking_jumper:
    bl      _asm_cpu_enter_user_space
    // bl      system_enable_interrupts_only_counter
    mov     r0, sp // saving current sp to bring it to the new mode
    cps	    #0x13
    mov     sp, r0
    pop     {pc}
