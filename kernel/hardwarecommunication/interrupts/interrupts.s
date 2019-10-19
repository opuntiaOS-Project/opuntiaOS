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
global irq_null

extern isr0_handler
extern isr1_handler
extern isr2_handler
extern isr3_handler
extern isr4_handler
extern isr5_handler
extern isr6_handler
extern isr7_handler
extern isr8_handler
extern isr9_handler
extern isr10_handler
extern isr11_handler
extern isr12_handler
extern isr13_handler
extern isr14_handler
extern isr15_handler
extern isr16_handler
extern isr17_handler
extern isr18_handler
extern isr19_handler
extern isr20_handler
extern isr21_handler
extern isr22_handler
extern isr23_handler
extern isr24_handler
extern isr25_handler
extern isr26_handler
extern isr27_handler
extern isr28_handler
extern isr29_handler
extern isr30_handler
extern isr31_handler
; IRQs
extern irq_handler_master
extern irq_handler_slave
extern irq_handler_null


isr0: 
    cli
    pusha
    call isr0_handler
    popa
    sti
    iret

isr1: 
    cli
    pusha
    call isr1_handler
    popa
    sti
    iret

isr2: 
    cli
    pusha
    call isr2_handler
    popa
    sti
    iret

isr3: 
    cli
    pusha
    call isr3_handler
    popa
    sti
    iret

isr4: 
    cli
    pusha
    call isr4_handler
    popa
    sti
    iret

isr5: 
    cli
    pusha
    call isr5_handler
    popa
    sti
    iret

isr6: 
    cli
    pusha
    call isr6_handler
    popa
    sti
    iret

isr7: 
    cli
    pusha
    call isr7_handler
    popa
    sti
    iret

isr8: 
    cli
    pusha
    call isr8_handler
    popa
    sti
    iret

isr9: 
    cli
    pusha
    call isr9_handler
    popa
    sti
    iret

isr10: 
    cli
    pusha
    call isr10_handler
    popa
    sti
    iret

isr11: 
    cli
    pusha
    call isr11_handler
    popa
    sti
    iret

isr12: 
    cli
    pusha
    call isr12_handler
    popa
    sti
    iret

isr13: 
    cli
    pusha
    call isr13_handler
    popa
    sti
    iret

isr14: 
    cli
    pusha
    call isr14_handler
    popa
    sti
    iret

isr15: 
    cli
    pusha
    call isr15_handler
    popa
    sti
    iret

isr16: 
    cli
    pusha
    call isr16_handler
    popa
    sti
    iret

isr17: 
    cli
    pusha
    call isr17_handler
    popa
    sti
    iret

isr18: 
    cli
    pusha
    call isr18_handler
    popa
    sti
    iret

isr19: 
    cli
    pusha
    call isr19_handler
    popa
    sti
    iret

isr20: 
    cli
    pusha
    call isr20_handler
    popa
    sti
    iret

isr21: 
    cli
    pusha
    call isr21_handler
    popa
    sti
    iret

isr22: 
    cli
    pusha
    call isr22_handler
    popa
    sti
    iret

isr23: 
    cli
    pusha
    call isr23_handler
    popa
    sti
    iret

isr24: 
    cli
    pusha
    call isr24_handler
    popa
    sti
    iret

isr25: 
    cli
    pusha
    call isr25_handler
    popa
    sti
    iret

isr26: 
    cli
    pusha
    call isr26_handler
    popa
    sti
    iret

isr27: 
    cli
    pusha
    call isr27_handler
    popa
    sti
    iret

isr28: 
    cli
    pusha
    call isr28_handler
    popa
    sti
    iret

isr29: 
    cli
    pusha
    call isr29_handler
    popa
    sti
    iret

isr30: 
    cli
    pusha
    call isr30_handler
    popa
    sti
    iret

isr31: 
    cli
    pusha
    call isr31_handler
    popa
    sti
    iret

irq0:
    cli
    pusha
    push byte 32
    call irq_handler_master
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq1:
    cli
    pusha
    push byte 33
    call irq_handler_master
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq2:
    cli
    pusha
    push byte 34
    call irq_handler_master
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq3:
    cli
    pusha
    push byte 35
    call irq_handler_master
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq4:
    cli
    pusha
    push byte 36
    call irq_handler_master
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq5:
    cli
    pusha
    push byte 37
    call irq_handler_master
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq6:
    cli
    pusha
    push byte 38
    call irq_handler_master
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq7:
    cli
    pusha
    push byte 39
    call irq_handler_master
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq8:
    cli
    pusha
    push byte 40
    call irq_handler_slave
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq9:
    cli
    pusha
    push byte 41
    call irq_handler_slave
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq10:
    cli
    pusha
    push byte 42
    call irq_handler_slave
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq11:
    cli
    pusha
    push byte 43
    call irq_handler_slave
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq12:
    cli
    pusha
    push byte 44
    call irq_handler_slave
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq13:
    cli
    pusha
    push byte 45
    call irq_handler_slave
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq14:
    cli
    pusha
    push byte 46
    call irq_handler_slave
    add esp, 4 ; delete byte
    popa
    sti
    iret
 
irq15:
    cli
    pusha
    push byte 47
    call irq_handler_slave
    add esp, 4 ; delete byte
    popa
    sti
    iret

irq_null:
    cli
    pusha
    push byte 0
    call irq_handler_null
    add esp, 4 ; delete byte
    popa
    sti
    iret