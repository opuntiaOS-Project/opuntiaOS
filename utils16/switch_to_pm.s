switch_to_pm:
    cli ; turn off interuptions
    lgdt [gdt_descriptor]
    mov ax, cr0
    or ax, 0x1
    mov cr0, ax
    call CODE_SEG:init_pm

init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000 ; stack's init
    mov esp, ebp
    
    call BEGIN_PM