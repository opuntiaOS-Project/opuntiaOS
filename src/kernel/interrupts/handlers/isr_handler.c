#include <isr_handler.h>

uint32_t isr_handler(uint8_t int_no, uint32_t esp) {
    isr_standart_handler(int_no);
    return esp;
}

void isr_standart_handler(uint8_t int_no) {
    const char *exception_messages[32] = { "Division by zero",
		"Debug",
		"Non-maskable interrupt",
		"Breakpoint",
		"Detected overflow",
		"Out-of-bounds",
		"ejfiowejfoiwejofijewoifjweoifjwoiejfoiwejfoiwejfiowejfoiwejfoiwejfoiwjfoiwejfoiwejfoiwejfoiwejfoiwejf", //Invalid opcode
		"No coprocessor",
		"Double fault",
		"Coprocessor segment overrun",
		"Bad TSS",
		"Segment not present",
		"Stack fault",
		"", // General protection fault
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
}
