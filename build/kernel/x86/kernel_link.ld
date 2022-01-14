SECTIONS
{
    . = 0xc0000000;
	__start = .;

	.text ALIGN(4K) : AT (ADDR(.text) - 0xc0000000 + 1M)
	{
		*(.opuntiaos_kernel_boot)
		*(.text)
	}

	.rodata ALIGN(4K) : AT (ADDR(.rodata) - 0xc0000000 + 1M)
	{
		*(.rodata)
	}

	.rodata.driver_init ALIGN(4K) : AT (ADDR(.rodata.driver_init) - 0xc0000000 + 1M)
	{
		_dirvers_init_start = .;
		*(SORT_BY_NAME(.driver_init_sections.*))
		_dirvers_init_end = .;
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