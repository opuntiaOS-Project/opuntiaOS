#include <isr_handler.h>
#include <mem/vmm/vmm.h>
#include <tasking/tasking.h> // TODO del that

uint32_t rcr2() {
	uint32_t val;
	asm volatile("movl %%cr2,%0" : "=r" (val));
	return val;
}

void isr_handler(trapframe_t *tf) {
    isr_standart_handler(tf);
}

void syscall_processor(trapframe_t *tf) {
	if (tf->int_no == 48) {
		uint32_t value = tf->eax;
		printd(value);
	}
	if (tf->int_no == 49) {
		char letter = tf->eax;
		char *txt = " \n";
		txt[0] = letter;
		printf(txt);
	}
	if (tf->int_no == 50) {
		printf("fork was called\n");
		tasking_fork();
	}
	if (tf->int_no == 51) {
		printf("exec was called\n");
		tasking_exec();
	}
}

void isr_standart_handler(trapframe_t *tf) {
	if (tf->int_no >= 48) {
		syscall_processor(tf);
		return;
	}
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
    printf(exception_messages[tf->int_no]);
    printd(tf->err);
	if (tf->int_no == 13) {
		while (1) {};
	}
    if (tf->int_no == 14) {
		if (tf->err == 5) {
			printh(tf->eip); printf("\n");
			printh(tf->esp); printf("\n");
			printh(tf->ebp); printf("\n");
			printf(" ");
			printh(rcr2());
			while(1) {}
		}
		printf(" ");
		printh(rcr2());
		vmm_page_fault_handler(tf->err, rcr2());
    }
}
