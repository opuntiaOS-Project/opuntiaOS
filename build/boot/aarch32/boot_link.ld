 SECTIONS
{
	boot_start = 0x80010000;
	. = boot_start;

	.text ALIGN(4K) :
	{
		*(.opuntiaos_load_section)
		*(.text)
	}

	.rodata ALIGN(4K) : 
	{
		*(.rodata)
	}

	.rodata.odt ALIGN(4K) :
	{
		_odt_phys = .;
		*(.odt)
		_odt_phys_end = .;
	}

	.data ALIGN(4K) : 
	{
		*(.data)
	}

	.bss ALIGN(4K) : 
	{
		*(COMMON)
		*(.bss)
	}

	.stack ALIGN(4K) :
	{
		STACK_SECONDARY_PHYZ_BASE = .;
		. += 0x1000;
		STACK_SECONDARY_PHYZ_TOP = .;

		STACK_PHYZ_BASE = .;
		. += 0x1000;
		STACK_PHYZ_TOP = .;
	}


	__end = .;

	.ARM.exidx : { *(.ARM.exidx) }

}

ENTRY(boot_start)