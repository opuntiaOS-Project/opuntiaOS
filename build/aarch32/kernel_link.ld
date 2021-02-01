 SECTIONS
{
	boot_start = 0x80010000;
	. = 0x80010000;
	.init :
	{
		*(.oneos_load_section)
		*(.init_code)
		*(.init_data)
	}

	phys_passed = 0x80100000;
	. = 0xc0000000;
	__start = .;

	.text BLOCK(4K) : AT (ADDR(.text) - 0xc0000000 + phys_passed)
	{
		*(.interrupt_vector_table)
		*(.text)
	}

	.rodata BLOCK(4K) : AT (ADDR(.rodata) - 0xc0000000 + phys_passed)
	{
		*(.rodata)
	}

	.data BLOCK(4K) : AT (ADDR(.data) - 0xc0000000 + phys_passed)
	{
		*(.data)
	}

	.bss BLOCK(4K) : AT (ADDR(.bss) - 0xc0000000 + phys_passed)
	{
		*(COMMON)
		*(.bss)
	}

	.stack BLOCK(4K) : AT (ADDR(.bss) - 0xc0000000 + phys_passed)
	{
		STACK_BASE = .;
		STACK_PHYZ_BASE = . - 0xc0000000 + phys_passed;
		. += 0x1000;
		STACK_TOP =  .;
		STACK_PHYZ_TOP = . - 0xc0000000 + phys_passed;

		STACK_SVC_BASE = .;
		. += 0x1000;
		STACK_SVC_TOP =  .;

		STACK_IRQ_BASE = .;
		. += 0x1000;
		STACK_IRQ_TOP =  .;

		STACK_ABORT_BASE = .;
		. += 0x1000;
		STACK_ABORT_TOP =  .;
	}

	__end = .;
}

ENTRY(boot_start)