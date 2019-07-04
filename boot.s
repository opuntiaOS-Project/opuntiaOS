[org 0x7c00]

mov [BOOT_DISK], dl ; saving boot disk

mov bp, 0x8000 ; stack init in 16bits
mov sp, bp

mov bx, MSG_REAL_MODE
call print_string
call switch_to_pm

jmp $

%include "utils16/print.s"
%include "utils16/disk_load.s"
%include "utils16/switch_to_pm.s"

%include "utils32/print.s"
%include "utils32/gdt.s"

[bits 32]
begin_pm:
    mov ebx, MSG_PROT_MODE
    call print_string_pm
    jmp $

MSG_REAL_MODE:
    db 'Starting in real mode', 0
MSG_PROT_MODE:
    db 'Switched to prot mode', 0

BOOT_DISK: db 0

times (510-($-$$)) db 0
db 0x55
db 0xaa