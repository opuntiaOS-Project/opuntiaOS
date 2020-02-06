#include <x86/gdt.h>

void lgdt(void* p, uint16_t size) {
    volatile uint16_t pd[3];
    pd[0] = size-1;
    pd[1] = (uint32_t)p;
    pd[2] = (uint32_t)p >> 16;
    asm volatile("lgdt (%0)" : : "r" (pd));
}

void gdt_setup() {
    gdt[SEG_KCODE] = SEG(SEGF_X|SEGF_R, 0, 0xffffffff, 0);
    gdt[SEG_KDATA] = SEG(SEGF_W, 0, 0xffffffff, 0);
    gdt[SEG_UCODE] = SEG(SEGF_X|SEGF_R, 0, 0xffffffff, DPL_USER);
    gdt[SEG_UDATA] = SEG(SEGF_W, 0, 0xffffffff, DPL_USER);
    lgdt(gdt, sizeof(gdt));
}