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
} __attribute__((packed)) context_t;

typedef struct {
    uint32_t sz;
    pdirectory_t* pdir;
    uint32_t pid;
    
    char *kstack;
    context_t *context; // context of kernel's registers
    trapframe_t *tf;
} __attribute__((packed)) proc_t;

proc_t proc[32];
uint32_t nxt_proc;

void run_proc();
void set_proc2();

#endif // __oneOS__X86__TASKING__TASKING_H