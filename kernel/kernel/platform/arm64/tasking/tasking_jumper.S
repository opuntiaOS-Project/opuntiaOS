.global _tasking_jumper
.global switch_to_context
.extern system_enable_interrupts
.extern _asm_cpu_enter_user_space

// void _tasking_jumper()
_tasking_jumper:
    bl _asm_cpu_enter_user_space
    bl system_enable_interrupts_only_counter
    ldr x0, [sp]
    add sp, sp, #0x8
    br x0
