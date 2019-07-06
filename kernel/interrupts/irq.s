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

global load_idt

extern irq0_handler
extern irq1_handler
extern irq2_handler
extern irq3_handler
extern irq4_handler
extern irq5_handler
extern irq6_handler
extern irq7_handler
extern irq8_handler
extern irq9_handler
extern irq10_handler
extern irq11_handler
extern irq12_handler
extern irq13_handler
extern irq14_handler
extern irq15_handler

irq0:
    pusha
    call irq0_handler
    popa
    iret
 
irq1:
    pusha
    call irq1_handler
    popa
    iret
 
irq2:
    pusha
    call irq2_handler
    popa
    iret
 
irq3:
    pusha
    call irq3_handler
    popa
    iret
 
irq4:
    pusha
    call irq4_handler
    popa
    iret
 
irq5:
    pusha
    call irq5_handler
    popa
    iret
 
irq6:
    pusha
    call irq6_handler
    popa
    iret
 
irq7:
    pusha
    call irq7_handler
    popa
    iret
 
irq8:
    pusha
    call irq8_handler
    popa
    iret
 
irq9:
    pusha
    call irq9_handler
    popa
    iret
 
irq10:
    pusha
    call irq10_handler
    popa
    iret
 
irq11:
    pusha
    call irq11_handler
    popa
    iret
 
irq12:
    pusha
    call irq12_handler
    popa
    iret
 
irq13:
    pusha
    call irq13_handler
    popa
    iret
 
irq14:
    pusha
    call irq14_handler
    popa
    iret
 
irq15:
    pusha
    call irq15_handler
    popa
    iret

load_idt:
    push edx
    mov edx, [esp+4]
    lidt [edx]
    sti
    pop edx
    ret