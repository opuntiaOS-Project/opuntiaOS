#include <types.h>

#define CODE_SEG 0x08
#define DATA_SEG 0x10
#define IDT_ENTRIES 256

#define IRQ_MASTER_OFFSET 32
#define IRQ_SLAVE_OFFSET 40

typedef struct {
    // TODO add segments support
    u_int8 int_no;
    u_int32 edi, esi, ebp, esp, ebx, edx, ecx, eax;
    u_int32 eip, cs, eflags, useresp, ss;
} regs_t;

u_int32* handlers[IDT_ENTRIES];

