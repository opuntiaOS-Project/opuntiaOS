[bits 16]

memory_map_entry:
base_address:
    resq 1
length:
    resq 1
type:
    resd 1
acpi_null:
    resd 1
memory_map_entry_end:

memory_map_size: dw 0

bios_get_memory_map:
    pushad
    xor ebx, ebx
    xor bp, bp
    mov edx, 'PAMS'
    mov eax, 0xe820
    mov ecx, 24
    int 0x15
    jc memory_map_error
    cmp eax, 'PAMS'
    jne memory_map_error
    test ebx, ebx
    je memory_map_error
    jmp memory_map_start
memory_map_next:
	mov	edx, 'PAMS'
	mov	ecx, 24
	mov	eax, 0xe820
	int	0x15
memory_map_start:
    jcxz memory_map_skip_entry
memory_map_common:
    mov ecx, [es:di + 8]
    test ecx, ecx
	jne memory_map_good_entry
    mov ecx, [es:di + 12]
    jecxz memory_map_skip_entry
memory_map_good_entry:
    inc bp
    add di, 24
memory_map_skip_entry:
    cmp ebx, 0
    jne memory_map_next
    jmp memory_map_done
memory_map_error:
    stc
memory_map_done:
    mov [memory_map_size], bp
    popad
    ret