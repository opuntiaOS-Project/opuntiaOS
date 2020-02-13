#include <tasking/tasking.h>
#include <fs/vfs.h>
#include <x86/idt.h>
#include <x86/gdt.h>
#include <x86/tss.h>

#define FL_IF 0x00000200

extern irq_return();
extern switch_contexts(context_t **old, context_t *new);

void switchuvm(proc_t *p) {
    gdt[SEG_TSS] = SEG_BG(SEGTSS_TYPE, &tss, sizeof(tss)-1, 0);
    tss.esp0 = p->kstack + 512;
    tss.ss0 = (SEG_KDATA << 3);
    // tss.iomap_offset = 0xffff;
    ltr(SEG_TSS << 3);
    vmm_switch_pdir(p->pdir);
}

void jmper() {
    // will jmp to trap frame
    return;
}

// Start proccess
// TODO split into functions
void run_proc() {
    nxt_proc = 0;
    proc_t *p = &proc[nxt_proc];
    char *code = vfs_read_file("/", "init.sys", 0, -1);
    vfs_element_t fstat = vfs_get_file_info("/", "init.sys");
    
    p->pdir = vmm_new_user_pdir();   
    
    vmm_copy_program_data(p->pdir, code, fstat.file_size);

    // TODO increase satck to PAGE_SIZE
    // allocating kernel stack
    p->kstack = kmalloc(512);
    char *sp = p->kstack + 512;
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
    p->tf->esp = 512;
    p->tf->eip = 0;

    context_t tmpr;
    context_t *tmp_ptr = &tmpr;

    switchuvm(p);
    switch_contexts(&tmp_ptr, p->context);
}
