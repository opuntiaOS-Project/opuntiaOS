SECTIONS
{
    . = 0xc0000000;
	__start = .;

	.text ALIGN(4K) : AT (ADDR(.text) - 0xc0000000 + 1M)
	{
		*(.oneos_kernel_boot)
		*(.text)
	}

	.rodata ALIGN(4K) : AT (ADDR(.rodata) - 0xc0000000 + 1M)
	{
		*(.rodata)
	}

	.data ALIGN(4K) : AT (ADDR(.data) - 0xc0000000 + 1M)
	{
		*(.data)
	}

	.bss ALIGN(4K) : AT (ADDR(.bss) - 0xc0000000 + 1M)
	{
		*(COMMON)
		*(.bss)
	}

	__end = .;
}