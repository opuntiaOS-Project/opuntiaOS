SECTIONS
{
	
    . = 0xc0000000;
	__start = .;

	.text BLOCK(4K) : AT (ADDR(.text) - 0xc0000000 + 1M)
	{
		*(.multiboot)
		*(.text)
	}

	.rodata BLOCK(4K) : AT (ADDR(.rodata) - 0xc0000000 + 1M)
	{
		*(.rodata)
	}

	.data BLOCK(4K) : AT (ADDR(.data) - 0xc0000000 + 1M)
	{
		*(.data)
	}

	.bss BLOCK(4K) : AT (ADDR(.bss) - 0xc0000000 + 1M)
	{
		*(COMMON)
		*(.bss)
	}

	__end = .;
}