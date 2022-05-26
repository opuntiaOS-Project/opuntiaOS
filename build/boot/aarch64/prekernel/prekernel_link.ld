/* PIC */
_pa_base = 0x0;

ENTRY(_pa_base)

SECTIONS
{
	. = _pa_base;
	prekernel_start_offset = .;

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

	prekernel_end_offset = .;
}