[org 0x7c00]

KERNEL_OFFSET equ 0x1000

mov [BOOT_DISK], dl ; saving boot disk

mov bp, 0x8000 ; stack init in 16bits
mov sp, bp

mov bx, MSG_REAL_MODE
call print_string
call load_kernel
call switch_to_pm

jmp $

%include "kernel/boot/utils16/print.s"
%include "kernel/boot/utils16/disk_load.s"
%include "kernel/boot/utils16/switch_to_pm.s"

%include "kernel/boot/utils32/print.s"
%include "kernel/boot/utils32/gdt.s"


[bits 16]
load_kernel:
    mov bx, MSG_KERNEL_LOAD
    call print_string

    mov bx, KERNEL_OFFSET
    mov dh, 20 ; sectors count to read
    mov dl, [BOOT_DISK]
    call disk_load
    ret

[bits 32]
begin_pm:
    mov ebx, MSG_PROT_MODE
    call print_string_pm
    call KERNEL_OFFSET
    jmp $

MSG_REAL_MODE:
    db 'Starting real mode', 0
MSG_PROT_MODE:
    db 'Switched to prot mode', 0
MSG_KERNEL_LOAD:
    db ' Loading kernel from drive', 0

BOOT_DISK: db 0

times (510-($-$$)) db 0
db 0x55
db 0xaa