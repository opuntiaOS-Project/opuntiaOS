#include <platform/x86/tss.h>
#include <platform/x86/gdt.h>
#include <mem/vmm/vmm.h>
#include <mem/kmalloc.h>

tss_t tss;

void ltr(uint16_t seg) {
    asm volatile("ltr %0" : : "r"(seg));
}

extern void jump_usermode();

char *kernel_stack;

// test function
// will be removed
// TODO might be unused
void switch_to_user_mode() {
    kernel_stack = (char*)kmalloc(VMM_PAGE_SIZE);
    gdt[SEG_TSS] = SEG_BG(SEGTSS_TYPE, &tss, sizeof(tss)-1, 0);
    tss.esp0 = (uint32_t)(kernel_stack + VMM_PAGE_SIZE - 1);
    tss.ss0 = (SEG_KDATA << 3);
    // tss.iomap_offset = 0xffff;
    ltr(SEG_TSS << 3);
    jump_usermode();
}

void user_mode_function() {
    while (1) {}
}