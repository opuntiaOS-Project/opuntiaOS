_pa_base = 0x0;
_va_base = 0xffff800000000000;

ENTRY(_va_base)

SECTIONS
{
    . = _va_base;

	__text_start = .;
	.text ALIGN(4K) : AT (ADDR(.text) - _va_base + _pa_base)
	{
		*(.opuntiaos_kernel_boot)
		*(.text)
		*(.text.*)
	}
	__text_end = .;

	__rodata_start = .;
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
	__rodata_end = .;

	__data_start = .;
	.data ALIGN(4K) : AT (ADDR(.data) - _va_base + _pa_base)
	{
		*(.data)
		*(.data.*)
	}
	__data_end = .;

	__bss_start = .;
	.bss ALIGN(4K) : AT (ADDR(.bss) - _va_base + _pa_base)
	{
		*(.bss)
		*(.bss.*)
		*(COMMON)
	}
	__bss_end = .;

	__stack_start = .;
	.stack ALIGN(4K) : AT (ADDR(.stack) - _va_base + _pa_base)
	{
		STACK_BASE = .;
		. += 0x1000;
		STACK_TOP =  .;
	}
	__stack_end = .;
	__end = .;
}