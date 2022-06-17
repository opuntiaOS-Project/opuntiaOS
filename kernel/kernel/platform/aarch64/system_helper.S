.global system_set_pdir
system_set_pdir:
    ic iallu
    dsb sy
    isb sy
    msr ttbr0_el1, x0
    msr ttbr1_el1, x1
    isb sy
    tlbi vmalle1
    isb sy
    ic iallu
    dsb sy
    isb sy
    ret