#ifndef __oneOS__X86__TASKING__TASKING_H
#define __oneOS__X86__TASKING__TASKING_H

#include <types.h>
#include <mem/vmm/vmm.h>
#include <x86/idt.h>

typedef struct {
    uint32_t edi;
    uint32_t esi;
    uint32_t ebx;
    uint32_t ebp;
    uint32_t eip;
} context_t;

typedef struct {
    uint32_t sz;
    pdirectory_t* pdir;
    uint32_t pid;
    
    uint32_t *kstack;
    context_t *context; // context of kernel's registers
    trapframe_t *tf;
} proc_t;

proc_t proc[32];
uint32_t nxt_proc;


void run_proc();

#endif // __oneOS__X86__TASKING__TASKING_H