#ifndef __oneOS__X86__TASKING__SCHED_H
#define __oneOS__X86__TASKING__SCHED_H

#include <types.h>
#include <mem/vmm/vmm.h>
#include <x86/idt.h>
#include <tasking/tasking.h>

#define CPU_CNT 1

#define SCHED_INT 20

typedef struct {
    char *kstack;
    context_t *scheduler; // context of sched's registers
} __attribute__((packed)) cpu_t;

extern cpu_t cpus[CPU_CNT];
extern proc_t *active_proc;

void scheduler_init();
void presched();
void presched_no_context();
void sched();

#endif // __oneOS__X86__TASKING__SCHED_H