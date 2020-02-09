#ifndef __oneOS__X86__TASKING__TASKING_H
#define __oneOS__X86__TASKING__TASKING_H

#include <types.h>
#include <mem/vmm/vmm.h>

typedef struct {
    uint32_t esp;
    uint32_t ebp;
} context_t;

typedef struct {
    uint32_t sz;
    pdirectory_t* pdir;
    uint32_t pid;
    
    uint32_t* kstack;
    context_t context; // context of kernel's registers
    
} proc_t;

#endif // __oneOS__X86__TASKING__TASKING_H