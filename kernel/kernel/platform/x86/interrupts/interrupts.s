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

%macro ADD_ISR 1
  global isr%1
  isr%1:
    push 0
    push %1
    jmp isr_common
%endmacro

%macro ADD_IRQ 2
  global isr%1
  isr%1:
    push 0
    push %2
    jmp irq_common
%endmacro

ADD_ISR 0
ADD_ISR 1
ADD_ISR 2
ADD_ISR 3
ADD_ISR 4
ADD_ISR 5
ADD_ISR 6
ADD_ISR 7
ADD_ISR 8
ADD_ISR 9
ADD_ISR 10
ADD_ISR 11
ADD_ISR 12
ADD_ISR 14
ADD_ISR 15
ADD_ISR 16
ADD_ISR 17
ADD_ISR 18
ADD_ISR 19
ADD_ISR 20
ADD_ISR 21
ADD_ISR 22
ADD_ISR 23
ADD_ISR 24
ADD_ISR 25
ADD_ISR 26
ADD_ISR 27
ADD_ISR 28
ADD_ISR 29
ADD_ISR 30
ADD_ISR 31


ADD_IRQ 0, 32
ADD_IRQ 1, 33
ADD_IRQ 2, 34
ADD_IRQ 3, 35
ADD_IRQ 4, 36
ADD_IRQ 5, 37
ADD_IRQ 6, 38
ADD_IRQ 7, 39
ADD_IRQ 8, 40
ADD_IRQ 9, 41
ADD_IRQ 10, 42
ADD_IRQ 11, 43
ADD_IRQ 12, 44
ADD_IRQ 13, 45
ADD_IRQ 14, 46
ADD_IRQ 15, 47

syscall:
    push 0
    push 0x80
    jmp  sys_common


