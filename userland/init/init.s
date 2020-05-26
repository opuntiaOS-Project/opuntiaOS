section .text

global main

main:
    mov esp, ebp
    mov ebx, 332

    mov eax, 0x02 ; fork
    int 0x80 ; syscall
    mov ebx, eax
    cmp eax, 0 
    jne exec ; starting new proc

loop: ; for main proc
    mov eax, 0x00 ; print
    ; int 0x80 ; syscall
    jmp loop
exec:
    mov ecx, pathToLaunch
    mov eax, 6 ; exec
    int 0x80 ; syscall
    jmp loop
    ; since that we launch a new prgram

pathToLaunch:  db "/bin/test", 0