_pa_base = 0x0;
_va_base = 0xffff800000000000;

ENTRY(_va_base)

SECTIONS
{
    . = _va_base;

	__text_start = .;
	.text ALIGN(4K) :
	{
		*(.opuntiaos_kernel_boot)
		*(.text)
		*(.text.*)
	}
	__text_end = .;

	__rodata_start = .;
	.rodata ALIGN(4K) :
	{
		*(.rodata)
		*(.rodata.*)
	}

	.rodata.driver_init ALIGN(4K) :
	{
		_dirvers_init_start = .;
		*(SORT_BY_NAME(.driver_init_sections.*))
		_dirvers_init_end = .;
	}
	__rodata_end = .;

	__data_start = .;
	.data ALIGN(4K) :
	{
		*(.data)
		*(.data.*)
	}
	__data_end = .;

	__got_start = .;
	.got ALIGN(4K) :
	{
		*(.got)
		*(.got.*)
	}
	__got_end = .;

	__bss_start = .;
	.bss ALIGN(4K) :
	{
		*(.bss)
		*(.bss.*)
		*(COMMON)
	}
	__bss_end = .;

	__stack_start = .;
	.stack ALIGN(4K) :
	{
		STACK_BASE = .;
		. += 0x1000;
		STACK_TOP =  .;
	}
	__stack_end = .;
	__end = .;
}