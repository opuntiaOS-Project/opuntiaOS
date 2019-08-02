#include <types.h>

#define CODE_SEG 0x08
#define DATA_SEG 0x10
#define IDT_ENTRIES 256

#define IRQ_MASTER_OFFSET 32
#define IRQ_SLAVE_OFFSET 40

typedef struct {
    // TODO add segments support
    uint8_t int_no;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t eip, cs, eflags, useresp, ss;
} regs_t;

uint32_t* handlers[IDT_ENTRIES];

