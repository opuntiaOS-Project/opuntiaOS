[org 0x7c00]

mov [BOOT_DISK], dl ; saving boot disk

mov bx, MSG_REAL_MODE
call print_string

jmp $

%include "utils16/print.s"

MSG_REAL_MODE:
    db 'Starting in real mode', 0

BOOT_DISK: db 0

times (510-($-$$)) db 0
db 0x55
db 0xaa