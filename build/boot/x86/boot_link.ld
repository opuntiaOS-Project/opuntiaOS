_pa_base = 0x1000;

ENTRY(_pa_base)

SECTIONS
{
	. = _pa_base;
	bootloader_start = .;

	.text ALIGN(4K) :
	{
		*(.opuntiaos_boot_text)
		*(.text)
		*(.text.*)
	}

	.rodata ALIGN(4K) :
	{
		*(.rodata)
		*(.rodata.*)
	}

	.data ALIGN(4K) :
	{
		*(.data)
		*(.data.*)
	}

	.bss ALIGN(4K) :
	{
		*(.bss)
		*(.bss.*)
		*(COMMON)
	}

	.stack ALIGN(4K) :
	{
		STACK_PHYZ_BASE = .;
		. += 0x1000;
		STACK_PHYZ_TOP = .;
	}
}