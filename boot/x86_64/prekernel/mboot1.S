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
global prekernel_entry

_start:
prekernel_entry:
    nop
    jmp prekernel_entry


// Print string to the bios display.
// edi -- pointer 
// esi -- size
print_string:
    nop
