#include <isr_handler.h>
#include <mem/vmm/vmm.h>

uint32_t isr_handler(uint8_t int_no, uint8_t more_inf, uint32_t cr2, uint32_t esp) {
    isr_standart_handler(int_no, more_inf, cr2);
    return esp;
}

void isr_standart_handler(uint8_t int_no, uint8_t more_inf, uint32_t cr2) {
    const char *exception_messages[32] = { "Division by zero",
		"Debug",
		"Non-maskable interrupt",
		"Breakpoint",
		"Detected overflow",
		"Out-of-bounds",
		"Invalid opcode", //Invalid opcode
		"No coprocessor",
		"Double fault",
		"Coprocessor segment overrun",
		"Bad TSS",
		"Segment not present",
		"Stack fault",
		"General protection fault", // General protection fault
		"Page fault",
		"Unknown interrupt",
		"Coprocessor fault",
		"Alignment check",
		"Machine check",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved",
		"Reserved"};
    printf(exception_messages[int_no]);
    printd(more_inf);
    if (int_no == 14) {
        vmm_page_fault_handler(more_inf, cr2);
    }
}
