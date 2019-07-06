#include "../../drivers/port/port.h"
#include "../../drivers/pic/pic.h"

#define CODE_SEG 0x08

struct IDT_entry {
    unsigned short int offset_lowerbits;
    unsigned short int selector;
    unsigned char zero;
    unsigned char type;
    unsigned short int offset_higherbits;
} IDT[256];

void idt_init();