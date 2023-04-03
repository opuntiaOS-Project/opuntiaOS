_pa_base = 0x80000000;

ENTRY(_pa_base)

SECTIONS
{
	. = _pa_base;
	PREKERNEL_START_OFFSET = .;

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

	.rela.text :
	{
		*(.rela.text)
	}

	.rela.bss :
	{
		*(.rela.bss)
	}

	.stack ALIGN(4K) :
	{
		PREKERNEL_STACK_BASE_OFFSET = .;
		. += 0x1000;
		PREKERNEL_STACK_TOP_OFFSET = .;
	}

	PREKERNEL_END_OFFSET = .;

	.devtree ALIGN(4K) :
	{
		EMBED_DEVTREE_START = .;
		*(.odt)
		EMBED_DEVTREE_END = .;
	}

	DEVTREE_END = .;

	.kernelelf ALIGN(4K) :
	{
		EMBED_KERNEL_START = .;
		*(.kernelelf)
		EMBED_KERNEL_END = .;
	}

	RAWIMAGE_END = .;
}