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

global syscall

extern isr_handler
extern irq_handler
extern sys_handler

global trap_return

int_no: db 0
com_inf: db 0
cr2_ch: db 0

isr_common:
    cli
    
    push ds
    push es
    push fs
    push gs
    pushad

    mov ax, 0x8 ; SEG_KDATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call isr_handler
    add esp, 4
    jmp trap_return

irq_common:
    cli
    
    push ds
    push es
    push fs
    push gs
    pushad

    mov ax, 0x8 ; SEG_KDATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call irq_handler
    add esp, 4
    jmp trap_return

sys_common:
    cli
    
    push ds
    push es
    push fs
    push gs
    pushad

    mov ax, 0x8 ; SEG_KDATA
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    
    push esp
    call sys_handler
    add esp, 4
    jmp trap_return

trap_return:
    popad
    pop gs
    pop fs
    pop es
    pop ds
    add esp, 0x8
    sti
    iret

isr0:
    push 0
    push 0
    jmp isr_common


isr1:
    push 0
    push 1
    jmp isr_common


isr2:
    push 0
    push 2
    jmp  isr_common


isr3:
    push 0
    push 3
    jmp  isr_common


isr4:
    push 0
    push 4
    jmp  isr_common


isr5:
    push 0
    push 5
    jmp  isr_common


isr6:
    push 0
    push 6
    jmp  isr_common


isr7:
    push 0
    push 7
    jmp  isr_common


isr8:
    push 0
    push 8
    jmp  isr_common


isr9:
    push 0
    push 9
    jmp  isr_common


isr10:
    push 0
    push 10
    jmp  isr_common


isr11:
    push 0
    push 11
    jmp  isr_common


isr12:
    push 0
    push 12
    jmp  isr_common


isr13:
    push 13
    jmp  isr_common


isr14:
    push 14
    jmp  isr_common


isr15:
    push 0
    push 15
    jmp  isr_common


isr16:
    push 0
    push 16
    jmp  isr_common


isr17:
    push 0
    push 17
    jmp  isr_common


isr18:
    push 0
    push 18
    jmp  isr_common


isr19:
    push 0
    push 19
    jmp  isr_common


isr20:
    push 0
    push 20
    jmp  isr_common


isr21:
    push 0
    push 21
    jmp  isr_common


isr22:
    push 0
    push 22
    jmp  isr_common


isr23:
    push 0
    push 23
    jmp  isr_common


isr24:
    push 0
    push 24
    jmp  isr_common


isr25:
    push 0
    push 25
    jmp  isr_common


isr26:
    push 0
    push 26
    jmp  isr_common


isr27:
    push 0
    push 27
    jmp  isr_common


isr28:
    push 0
    push 28
    jmp  isr_common


isr29:
    push 0
    push 29
    jmp  isr_common


isr30:
    push 0
    push 30
    jmp  isr_common


isr31:
    push 0
    push 31
    jmp  isr_common


irq0:
    push 0
    push 32
    jmp  irq_common


irq1:
    push 0
    push 33
    jmp  irq_common


irq2:
    push 0
    push 34
    jmp  irq_common


irq3:
    push 0
    push 35
    jmp  irq_common


irq4:
    push 0
    push 36
    jmp  irq_common


irq5:
    push 0
    push 37
    jmp  irq_common


irq6:
    push 0
    push 38
    jmp  irq_common


irq7:
    push 0
    push 39
    jmp  irq_common


irq8:
    push 0
    push 40
    jmp  irq_common


irq9:
    push 0
    push 41
    jmp  irq_common


irq10:
    push 0
    push 42
    jmp  irq_common


irq11:
    push 0
    push 43
    jmp  irq_common


irq12:
    push 0
    push 44
    jmp  irq_common


irq13:
    push 0
    push 45
    jmp  irq_common


irq14:
    push 0
    push 46
    jmp  irq_common


irq15:
    push 0
    push 47
    jmp  irq_common

syscall:
    push 0
    push 0x80
    jmp  sys_common


