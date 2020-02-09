global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31
; IRQs
global irq0
global irq1
global irq2
global irq3
global irq4
global irq5
global irq6
global irq7
global irq8
global irq9
global irq10
global irq11
global irq12
global irq13
global irq14
global irq15

extern isr_handler
extern irq_handler

int_no: db 0
com_inf: db 0
cr2_ch: db 0

isr_common:
    cli
    pushad
    push ds
    push es
    push fs
    push gs
    push esp
    xor eax, eax
    mov eax, cr2
    push eax ; for paging
    mov ax, [com_inf]
    push eax
    mov ax, [int_no]
    push eax
    call isr_handler
    mov esp, eax
    pop gs
    pop fs
    pop es
    pop ds
    popad
    sti
    iret

irq_common:
    cli
    pushad
    push ds
    push es
    push fs
    push gs
    push esp
    xor eax, eax
    mov ax, [int_no]
    push eax
    call irq_handler
    mov esp, eax
    pop gs
    pop fs
    pop es
    pop ds
    popad
    sti
    iret

isr0:
    mov byte[int_no], 0
    jmp isr_common


isr1:
    mov byte[int_no], 1
    jmp isr_common


isr2:
    mov byte[int_no], 2
    jmp  isr_common


isr3:
    mov byte[int_no], 3
    jmp  isr_common


isr4:
    mov byte[int_no], 4
    jmp  isr_common


isr5:
    mov byte[int_no], 5
    jmp  isr_common


isr6:
    mov byte[int_no], 6
    jmp  isr_common


isr7:
    mov byte[int_no], 7
    jmp  isr_common


isr8:
    mov byte[int_no], 8
    jmp  isr_common


isr9:
    mov byte[int_no], 9
    jmp  isr_common


isr10:
    mov byte[int_no], 10
    jmp  isr_common


isr11:
    mov byte[int_no], 11
    jmp  isr_common


isr12:
    mov byte[int_no], 12
    jmp  isr_common


isr13:
    push eax
    mov eax, [esp+4]
    mov [com_inf], al
    pop eax
    add esp, 4
    mov byte[int_no], 13
    jmp  isr_common


isr14:
    push eax
    mov eax, [esp+4]
    mov [com_inf], al
    pop eax
    add esp, 4
    mov byte[int_no], 14
    jmp  isr_common


isr15:
    mov byte[int_no], 15
    jmp  isr_common


isr16:
    mov byte[int_no], 16
    jmp  isr_common


isr17:
    mov byte[int_no], 17
    jmp  isr_common


isr18:
    mov byte[int_no], 18
    jmp  isr_common


isr19:
    mov byte[int_no], 19
    jmp  isr_common


isr20:
    mov byte[int_no], 20
    jmp  isr_common


isr21:
    mov byte[int_no], 21
    jmp  isr_common


isr22:
    mov byte[int_no], 22
    jmp  isr_common


isr23:
    mov byte[int_no], 23
    jmp  isr_common


isr24:
    mov byte[int_no], 24
    jmp  isr_common


isr25:
    mov byte[int_no], 25
    jmp  isr_common


isr26:
    mov byte[int_no], 26
    jmp  isr_common


isr27:
    mov byte[int_no], 27
    jmp  isr_common


isr28:
    mov byte[int_no], 28
    jmp  isr_common


isr29:
    mov byte[int_no], 29
    jmp  isr_common


isr30:
    mov byte[int_no], 30
    jmp  isr_common


isr31:
    mov byte[int_no], 31
    jmp  isr_common


irq0:
    mov byte[int_no], 32
    jmp  irq_common


irq1:
    mov byte[int_no], 33
    jmp  irq_common


irq2:
    mov byte[int_no], 34
    jmp  irq_common


irq3:
    mov byte[int_no], 35
    jmp  irq_common


irq4:
    mov byte[int_no], 36
    jmp  irq_common


irq5:
    mov byte[int_no], 37
    jmp  irq_common


irq6:
    mov byte[int_no], 38
    jmp  irq_common


irq7:
    mov byte[int_no], 39
    jmp  irq_common


irq8:
    mov byte[int_no], 40
    jmp  irq_common


irq9:
    mov byte[int_no], 41
    jmp  irq_common


irq10:
    mov byte[int_no], 42
    jmp  irq_common


irq11:
    mov byte[int_no], 43
    jmp  irq_common


irq12:
    mov byte[int_no], 44
    jmp  irq_common


irq13:
    mov byte[int_no], 45
    jmp  irq_common


irq14:
    mov byte[int_no], 46
    jmp  irq_common


irq15:
    mov byte[int_no], 47
    jmp  irq_common
