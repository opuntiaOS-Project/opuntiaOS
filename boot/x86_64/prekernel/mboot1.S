MBOOT_PAGE_ALIGN    equ 1<<0
MBOOT_MEM_INFO      equ 1<<1
MBOOT_HEADER_MAGIC  equ 0x1badb002
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

section .multiboot
dd MBOOT_HEADER_MAGIC
dd MBOOT_HEADER_FLAGS
dd MBOOT_CHECKSUM
dd 0x00000000; header_addr
dd 0x00000000; load_addr
dd 0x00000000; load_end_addr
dd 0x00000000; bss_end_addr
dd 0x00000000; entry_addr
dd 0x00000000; mode_type
dd 0x00000000; width
dd 0x00000000; height
dd 0x00000000; depth

[bits 32]
section .opuntiaos_boot_text

extern main

global _start
_start:
global prekernel_entry
prekernel_entry:
    cli
    cld

    ; Save entry point to one of callee-saved regs.
    mov esi, ecx

    ; PAE is required for x86_64
    call check_for_pae

    ; Checking for long mode.
    call check_for_long_mode

    ; x86_64 required paging enabled, thus we have to map 128MB of physical memmory
    ; for booting the kernel without any problems. 1 + 1 + 1 + 64.
    call setup_tables

    ; Enabling long mode with a 32-bit compatibility submode.
    call enable_long_mode

    ; Just give control to jumper.
    jmp jump_to_entry64

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


setup_tables:
    push ebp
    mov ebp, esp
    push edi

    mov edi, table0
    xor eax, eax
    mov ecx, 4096
    rep stosd

    mov edi, table1
    xor eax, eax
    mov ecx, 4096
    rep stosd

    mov edi, table2
    xor eax, eax
    mov ecx, 4096
    rep stosd

    mov eax, table1
    add eax, 0x3
    mov DWORD [table0], eax

    mov eax, table2
    add eax, 0x3
    mov DWORD [table1], eax
    
    mov edi, table2
    mov eax, 0x83
    mov ecx, 512
.table_fillup_last:
    mov DWORD [edi], eax
    add eax, 0x200000
    add edi, 8
    loop .table_fillup_last

    pop edi
    pop ebp
    ret


enable_long_mode:
    push ebp
    mov ebp, esp

    ; Setting table
    mov eax, table0
    mov cr3, eax

    ; Enabling PAE
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Enabling Long Mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    mov eax, cr0
    or eax, 1 << 31
    mov cr0, eax

    pop ebp
    ret


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


; Access bits
PRESENT        equ 1 << 7
NOT_SYS        equ 1 << 4
EXEC           equ 1 << 3
DC             equ 1 << 2
RW             equ 1 << 1
ACCESSED       equ 1 << 0
 
; Flags bits
GRAN_4K       equ 1 << 7
SZ_32         equ 1 << 6
LONG_MODE     equ 1 << 5

align 32
gdt_begin:
gdt_null:
    dq 0x0
gdt_code:
    dd 0xffff
    db 0x0
    db PRESENT | NOT_SYS | EXEC | RW
    db GRAN_4K | LONG_MODE | 0xF
    db 0x0
gdt_data:
    dd 0xffff
    db 0x0
    db PRESENT | NOT_SYS | RW
    db GRAN_4K | SZ_32 | 0xF
    db 0x0
gdt_tss:
    dd 0x00000068
    dd 0x00CF8900
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_begin - 1
    dd gdt_begin
    dd 0x0

CODE_SEG equ gdt_code - gdt_begin
DATA_SEG equ gdt_data - gdt_begin

jump_to_entry64:
    lgdt [gdt_descriptor]
    jmp CODE_SEG:main_entry64

[bits 64]
main_entry64:
    cli
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    mov edi, esi
    mov esi, ebx
    call main
    hlt

welcome_msg:
    db "Welcome to x86-64 on opuntiaOS", 0

pae_unsupported_msg:
    db "Required PAE Feature is unavailable, stopping...", 0

long_mode_unsupported_msg:
    db "Required Long Mode Feature is unavailable, stopping...", 0

section .bss

table0:
    align 4096
    resb 4096

table1:
    align 4096
    resb 4096

table2:
    align 4096
    resb 4096
