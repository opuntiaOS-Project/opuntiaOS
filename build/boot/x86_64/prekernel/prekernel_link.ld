/* PIC */
_pa_base = 1M;

/* ENTRY(_pa_base) */

SECTIONS
{
	. = _pa_base;
	PREKERNEL_START = .;

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

	.bss :
	{
		*(.bss)
		*(.bss.*)
		*(COMMON)
	}

	.stack ALIGN(4K) :
	{
		PREKERNEL_STACK_BASE = .;
		. += 0x1000;
		PREKERNEL_STACK_TOP = .;
	}

	PREKERNEL_END = .;

	.kernelelf ALIGN(4K) :
	{
		EMBED_KERNEL_START = .;
		*(.kernelelf)
		EMBED_KERNEL_END = .;
	}

	RAWIMAGE_END = .;
}