section .text
; push ebp
mov esp, ebp
mov ebx, 332
int 50 ; fork
int 48 ; printing eax
cmp eax, 0 
jne exec ; starting in new proc
loop: ; for main proc
    int 48
    jmp loop
exec:
    mov ecx, pathToLaunch
    int 51 ; exec
    jmp loop
    ; since that we lanch a new prgram

pathToLaunch:  db "sec.sys", 0