SECTIONS
{
	phys_passed = 0x80100000;
	phys_start = 0xc0000000;
	. = 0xc0000000;
	__start = .;

	.text ALIGN(4K) : AT (ADDR(.text) - 0xc0000000 + phys_passed)
	{
		*(.interrupt_vector_table)
		*(.text)
	}

	.rodata ALIGN(4K) : AT (ADDR(.rodata) - 0xc0000000 + phys_passed)
	{
		*(.rodata)
	}

	.rodata.driver_init ALIGN(4K) : AT (ADDR(.rodata.driver_init) - 0xc0000000 + phys_passed)
	{
		_dirvers_init_start = .;
		*(SORT_BY_NAME(.driver_init_sections.*))
		_dirvers_init_end = .;
	}

	.data ALIGN(4K) : AT (ADDR(.data) - 0xc0000000 + phys_passed)
	{
		*(.data)
	}

	.bss ALIGN(4K) : AT (ADDR(.bss) - 0xc0000000 + phys_passed)
	{
		*(COMMON)
		*(.bss)
	}

	.stack ALIGN(4K) : AT (ADDR(.stack) - 0xc0000000 + phys_passed)
	{
		STACK_SECONDARY_BASE = .;
		STACK_SECONDARY_PHYZ_BASE = . - 0xc0000000 + phys_passed;
		. += 0x1000;
		STACK_SECONDARY_TOP =  .;
		STACK_SECONDARY_PHYZ_TOP = . - 0xc0000000 + phys_passed;

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

		STACK_UNDEFINED_BASE = .;
		. += 0x1000;
		STACK_UNDEFINED_TOP =  .;
	}

	__end = .;

	.ARM.exidx : { *(.ARM.exidx) }

}

ENTRY(phys_start)