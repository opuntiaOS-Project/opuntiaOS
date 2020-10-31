global setjmp
setjmp:
    mov eax, [esp+4]
    mov [eax], ebx
    mov [eax+4], esi
    mov [eax+8], edi
    mov [eax+12], ebp
    lea ecx, [esp+4]
    mov [eax+16], ecx
    mov ecx, [esp]
    mov [eax+20], ecx
    xor eax, eax
    ret

global longjmp
longjmp:
    mov edx, [esp+4]
    mov eax, [esp+8]
    mov ebx, [edx]
    mov esi, [edx+4]
    mov edi, [edx+8]
    mov ebp, [edx+12]
    mov esp, [edx+16]
    mov ecx, [edx+20]
    test eax, eax
    jnz success
    mov eax, 1
success:
    jmp ecx
