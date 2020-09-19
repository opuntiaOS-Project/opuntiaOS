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
    ; mov eax, 0x00 ; print
    mov eax, 0x01 ; exit
    mov ebx, 0x02
    int 0x80 ; syscall
    jmp loop
exec:
    mov ebx, pathToLaunch
    mov eax, 11 ; exec
    int 0x80 ; syscall
    jmp loop
    ; since that we launch a new prgram

pathToLaunch:  db "/bin/disco", 0