.extern load_boot_cpu
.extern load_secondary_cpu

.global _start
_start:
    ldr     r0, =load_boot_cpu
    mov     pc, r0
1:
    b       1b
.size _start, . - _start

.global _start_secondary_cpu
_start_secondary_cpu:
    ldr     r0, =load_secondary_cpu
    mov     pc, r0
2:
    b       2b

.global jump_to_kernel
jump_to_kernel:
    isb
    mov     r5, #0xc0000000
    mov     pc, r5
3:
    b       3b
