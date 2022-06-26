/* PIC */
_pa_base = 1M;

/* ENTRY(_pa_base) */

SECTIONS
{
	. = _pa_base;
	PREKERNEL_START_OFFSET = .;

	.multiboot :
	{
		*(.multiboot)
	}
	
	.text :
	{
		*(.opuntiaos_boot_text)
		*(.text)
		*(.text.*)
	}

	.rodata :
	{
		*(.rodata)
		*(.rodata.*)
	}

	.data :
	{
		*(.data)
		*(.data.*)
	}

	.got :
	{
		*(.got)
		*(.got.*)
	}

	.bss :
	{
		*(.bss)
		*(.bss.*)
		*(COMMON)
	}

	.stack ALIGN(4K) :
	{
		PREKERNEL_STACK_BASE_OFFSET = .;
		. += 0x1000;
		PREKERNEL_STACK_TOP_OFFSET = .;
	}

	PREKERNEL_END_OFFSET = .;
}