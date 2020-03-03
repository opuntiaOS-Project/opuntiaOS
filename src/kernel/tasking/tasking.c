#include <tasking/tasking.h>
#include <fs/vfs.h>
#include <x86/idt.h>
#include <x86/gdt.h>
#include <x86/tss.h>
#include <tasking/sched.h>

#define FL_IF 0x00000200

extern irq_return();
extern switch_contexts(context_t **old, context_t *new);

void switchuvm(proc_t *p) {
    gdt[SEG_TSS] = SEG_BG(SEGTSS_TYPE, &tss, sizeof(tss)-1, 0);
    tss.esp0 = p->kstack + VMM_PAGE_SIZE;
    tss.ss0 = (SEG_KDATA << 3);
    // tss.iomap_offset = 0xffff;
    active_proc = p;
    ltr(SEG_TSS << 3);
    vmm_switch_pdir(p->pdir);
}

void jmper() {
    // jump to trap frame
    return;
}

// Start proccess
// TODO split into functions
void run_proc() {
    nxt_proc = 0;
    proc_t *p = &proc[nxt_proc];
    uint8_t *code = vfs_read_file("/", "main.sys", 0, -1);
    vfs_element_t fstat = vfs_get_file_info("/", "main.sys");

    p->pdir = vmm_new_user_pdir();   
    
    vmm_copy_program_data(p->pdir, code, fstat.file_size);

    // allocating kernel stack
    p->kstack = kmalloc(VMM_PAGE_SIZE);
    char *sp = p->kstack + VMM_PAGE_SIZE;
    sp -= sizeof(*p->tf);
    p->tf = (trapframe_t *)sp;
    sp -= 4;
    *(uint32_t*)sp = (uint32_t)irq_return;
    sp -= sizeof(*p->context);
    p->context = (context_t*)sp;
    memset(p->context, 0, sizeof(*p->context));
    p->context->eip = (uint32_t)jmper;

    memset(p->tf, 0, sizeof(*p->tf));
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->ebp = VMM_PAGE_SIZE - 1;
    p->tf->esp = VMM_PAGE_SIZE - 1;
    p->tf->eip = 0;

    context_t tmpr;
    context_t *tmp_ptr = &tmpr;

    switchuvm(p);
    switch_contexts(&tmp_ptr, p->context);
}

// TODO tmp solution to launch the second proc
// Will be deleted 
void set_proc2() {

    // setup_scheduler();

    nxt_proc = 1;
    proc_t *p = &proc[nxt_proc];
    char *code = vfs_read_file("/", "init2.sys", 0, -1);
    vfs_element_t fstat = vfs_get_file_info("/", "init2.sys");
    
    p->pdir = vmm_new_user_pdir();   
    
    vmm_copy_program_data(p->pdir, code, fstat.file_size);

    // allocating kernel stack
    p->kstack = kmalloc(VMM_PAGE_SIZE);
    char *sp = p->kstack + VMM_PAGE_SIZE;
    sp -= sizeof(*p->tf);
    p->tf = (trapframe_t *)sp;
    sp -= 4;
    *(uint32_t*)sp = (uint32_t)irq_return;
    sp -= sizeof(*p->context);
    p->context = (context_t*)sp;
    memset(p->context, 0, sizeof(*p->context));
    p->context->eip = (uint32_t)jmper;

    memset(p->tf, 0, sizeof(*p->tf));
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->ebp = VMM_PAGE_SIZE - 1;
    p->tf->esp = VMM_PAGE_SIZE - 1;
    p->tf->eip = 0;
}
