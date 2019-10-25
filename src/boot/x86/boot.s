[org 0x7c00]

KERNEL_OFFSET equ 0x1000
KERNEL_SIZE equ 32 ; (KBytes) 32KB
MEMORY_MAP_REGION equ 0xA00

mov [BOOT_DISK], dl ; saving boot disk

mov bp, 0x8000 ; stack init in 16bits
mov sp, bp

mov bx, MSG_REAL_MODE
call print_string

mov eax, 0
mov es, ax
mov di, MEMORY_MAP_REGION
call bios_get_memory_map

call load_kernel
call switch_to_pm
jmp $

%include "src/boot/x86/utils16/print.s"
%include "src/boot/x86/utils16/smm.s"
%include "src/boot/x86/utils16/disk_load.s"
%include "src/boot/x86/utils16/switch_to_pm.s"

%include "src/boot/x86/utils32/print.s"
%include "src/boot/x86/utils32/gdt.s"


[bits 16]
load_kernel:
    mov bx, MSG_KERNEL_LOAD
    call print_string

    mov bx, KERNEL_OFFSET
    mov dh, 50
    mov dl, [BOOT_DISK]
    call disk_load
    ret

[bits 32]
begin_pm:
    mov ebx, MSG_PROT_MODE
    call print_string_pm
    mov eax, dword [memory_map_size]
    mov [MEM_DESC], eax
    mov eax, dword KERNEL_SIZE
    mov [MEM_DESC+2], eax
    push dword MEM_DESC
    call KERNEL_OFFSET
    jmp $

MSG_REAL_MODE:
    db 'Starting real mode', 0
MSG_PROT_MODE:
    db 'Switched to prot mode', 0
MSG_KERNEL_LOAD:
    db ' Loading kernel from drive', 0

BOOT_DISK: db 0
MEM_DESC:
    dw 0x00 ; memory size
    dw 0x00 ; kernel size 


times (510-($-$$)) db 0
db 0x55
db 0xaa