#include "../../utils/types.h"

#define CODE_SEG 0x08
#define IDT_ENTRIES 256

#define IRQ_MASTER_OFFSET 32
#define IRQ_SLAVE_OFFSET 40

u_int32* handlers[IDT_ENTRIES];