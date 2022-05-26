.section ".opuntiaos_boot_text"
prekernel_header:
    // UEFI Signature
	ccmp x18, #0, #0xd, pl
    b prekernel_entry
    // TODO: UEFI header here

    

prekernel_entry:
    b prekernel_entry