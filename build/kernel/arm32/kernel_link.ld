_pa_base = 0x80100000;
_va_base = 0xc0000000;

ENTRY(_va_base)

SECTIONS
{
	. = _va_base;

	__text_start = .;
	.text ALIGN(4K) : AT (ADDR(.text) - _va_base + _pa_base)
	{
		*(.interrupt_vector_table)
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
		_drivers_init_start = .;
		*(SORT_BY_NAME(.driver_init_sections.*))
		_drivers_init_end = .;
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
		STACK_SECONDARY_BASE = .;
		. += 0x1000;
		STACK_SECONDARY_TOP =  .;

		STACK_BASE = .;
		. += 0x1000;
		STACK_TOP =  .;

		STACK_SVC_BASE = .;
		. += 0x1000;
		STACK_SVC_TOP =  .;

		STACK_IRQ_BASE = .;
		. += 0x1000;
		STACK_IRQ_TOP =  .;

		STACK_ABORT_BASE = .;
		. += 0x1000;
		STACK_ABORT_TOP =  .;

		STACK_UNDEFINED_BASE = .;
		. += 0x1000;
		STACK_UNDEFINED_TOP =  .;
	}
	__stack_end = .;

	.ARM.exidx : { *(.ARM.exidx) }

	__end = .;
}