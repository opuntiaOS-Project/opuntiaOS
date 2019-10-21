for i in range(32):
    print("""
isr{0}:
    cli
    pusha
    push ds
    push es
    push fs
    push gs
    push esp
    push byte {0}
    jmp  isr_common_bottom
    """.format(i)
)


for i in range(32, 40):
    print("""
irq{0}:
    cli
    pusha
    push ds
    push es
    push fs
    push gs
    push esp
    push byte {1}
    jmp  irq_master_common_bottom
    """.format(i-32, i)
)


for i in range(40, 48):
    print("""
irq{0}:
    cli
    pusha
    push ds
    push es
    push fs
    push gs
    push esp
    push byte {1}
    jmp  irq_slave_common_bottom
    """.format(i-32, i)
)
