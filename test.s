mov byte[dev], 5
int 0x01
mov eax, [dev]
ret
dev: db 0
