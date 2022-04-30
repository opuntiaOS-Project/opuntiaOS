.extern stage3

.section ".interrupt_vector_table"
.global trap_return
.global vector_table
aarch64_entry:
    ldr x3, =stage3
    br x3
aarch64_entry_loop:
    b aarch64_entry_loop

