_pa_base = 0x80010000;

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

	.rodata.odt ALIGN(4K) :
	{
		_odt_phys = .;
		*(.odt)
		_odt_phys_end = .;
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
		STACK_SECONDARY_PHYZ_BASE = .;
		. += 0x1000;
		STACK_SECONDARY_PHYZ_TOP = .;

		STACK_PHYZ_BASE = .;
		. += 0x1000;
		STACK_PHYZ_TOP = .;
	}

	.ARM.exidx : { *(.ARM.exidx) }
}