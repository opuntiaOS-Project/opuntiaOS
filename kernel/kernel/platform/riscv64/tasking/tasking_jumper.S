.global _tasking_jumper
.global switch_to_context
.extern system_enable_interrupts
.extern _asm_cpu_enter_user_space

// void _tasking_jumper()
_tasking_jumper:
    call _asm_cpu_enter_user_space
    call system_enable_interrupts_only_counter
    ld a0, 0(sp)
    addi sp, sp, 8
    jr a0
