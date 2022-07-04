global switch_contexts
global switch_to_context

switch_contexts:
    mov eax, [esp+4] ; old context
    mov edx, [esp+8] ; new context

    push ebp
    push ebx
    push esi
    push edi

    mov [eax], esp
    mov esp, edx

    pop edi
    pop esi
    pop ebx
    pop ebp

    ret

switch_to_context:
    mov edx, [esp+4] ; new context

    mov esp, edx

    pop edi
    pop esi
    pop ebx
    pop ebp

    ret