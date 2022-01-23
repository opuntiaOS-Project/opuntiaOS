_pa_base = 1M;
_va_base = 0xc0000000;

ENTRY(_va_base)

SECTIONS
{
    . = _va_base;

	.text ALIGN(4K) : AT (ADDR(.text) - _va_base + _pa_base)
	{
		*(.opuntiaos_kernel_boot)
		*(.text)
		*(.text.*)
	}

	.rodata ALIGN(4K) : AT (ADDR(.rodata) - _va_base + _pa_base)
	{
		*(.rodata)
		*(.rodata.*)
	}

	.rodata.driver_init ALIGN(4K) : AT (ADDR(.rodata.driver_init) - _va_base + _pa_base)
	{
		_dirvers_init_start = .;
		*(SORT_BY_NAME(.driver_init_sections.*))
		_dirvers_init_end = .;
	}

	.data ALIGN(4K) : AT (ADDR(.data) - _va_base + _pa_base)
	{
		*(.data)
		*(.data.*)
	}

	.bss ALIGN(4K) : AT (ADDR(.bss) - _va_base + _pa_base)
	{
		*(.bss)
		*(.bss.*)
		*(COMMON)
	}

	.stack ALIGN(4K) : AT (ADDR(.stack) - _va_base + _pa_base)
	{
		STACK_BASE = .;
		. += 0x1000;
		STACK_TOP =  .;
	}
}