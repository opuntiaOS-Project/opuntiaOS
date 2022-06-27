MBOOT_PAGE_ALIGN    equ 1<<0
MBOOT_MEM_INFO      equ 1<<1
MBOOT_HEADER_MAGIC  equ 0x1badb002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

section .multiboot
dd MBOOT_HEADER_MAGIC
dd MBOOT_HEADER_FLAGS
dd MBOOT_CHECKSUM

; Memory info location
dd 0x00000000; Header
dd 0x00000000; Load begin
dd 0x00000000; Load end
dd 0x00000000; Bss end
dd 0x00000000; Entry point

[bits 32]
section .opuntiaos_boot_text
global _start
_start:
global prekernel_entry
prekernel_entry:
    cli
    cld

    push welcome_msg
    call early_boot_print_string
    add esp, 4

    ; PAE is required for x86_64
    call check_for_pae

    ; Checking for long mode.
    call check_for_long_mode

    ; x86_64 required paging enabled, thus we have to map 128MB of physical memmory
    ; for booting the kernel without any problems. 1 + 1 + 1 + 64.


_prekernel_hand:
    jmp _prekernel_hand


CPUID_FEATURE_PAE equ (1 << 6)
check_for_pae:
    push ebp
    mov ebp, esp
    push ebx

    mov eax, 0x1
    cpuid
    test edx, CPUID_FEATURE_PAE
    jz pae_unsupported

    pop ebx
    pop ebp
    ret

pae_unsupported:
    push pae_unsupported_msg
    call early_boot_print_string
    hlt

CPUID_FEATURE_LONG_MODE equ (1 << 29)
check_for_long_mode:
    push ebp
    mov ebp, esp
    push ebx

    mov eax, 0x80000001
    cpuid
    test edx, CPUID_FEATURE_LONG_MODE
    jz long_mode_unsupported

    pop ebx
    pop ebp
    ret

long_mode_unsupported:
    push long_mode_unsupported_msg
    call early_boot_print_string
    hlt

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

global early_boot_print_string
early_boot_print_string:
    push ebp
    mov ebp, esp

    push esi
    push ecx

    mov esi, [ebp+8]
    mov ecx, VIDEO_MEMORY
    mov ah, WHITE_ON_BLACK

_print_string_loop:
    lodsb; Load from esi to al then increment esi

    test al, al
    jz _print_string_end

    mov [ecx], ax
    add ecx, 2

    jmp _print_string_loop

_print_string_end:
    mov eax, esi
    sub eax, [ebp+8]

    pop ecx
    pop esi

    mov esp, ebp
    pop ebp
    ret


welcome_msg:
    db "Welcome to x86-64 on opuntiaOS", 0

pae_unsupported_msg:
    db "Required PAE Feature is unavailable, stopping...", 0

long_mode_unsupported_msg:
    db "Required Long Mode Feature is unavailable, stopping...", 0