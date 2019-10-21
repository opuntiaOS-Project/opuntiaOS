[bits 32]

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

print_string_pm:
    pusha
    mov edx, VIDEO_MEMORY
print_string_pm_cycle:
    cmp [ebx], BYTE 0
    je print_string_pm_end
    mov ah, WHITE_ON_BLACK
    mov al, [ebx]
    mov [edx], ax

    add ebx, 1
    add edx, 2

    jmp print_string_pm_cycle
print_string_pm_end:
    popa
    ret