#include <tasking/tasking.h>
#include <fs/vfs.h>
#include <x86/idt.h>
#include <x86/gdt.h>
#include <x86/tss.h>
#include <tasking/sched.h>
#include <mem/malloc.h>

#define FL_IF 0x00000200

extern void trap_return();
extern void switch_contexts(context_t **old, context_t *new);

static int nxtpid = 0;

// switching the page dir and tss to the current proc
void switchuvm(proc_t *p) {
    gdt[SEG_TSS] = SEG_BG(SEGTSS_TYPE, &tss, sizeof(tss)-1, 0);
    tss.esp0 = (uint32_t)(p->kstack + VMM_PAGE_SIZE);
    tss.ss0 = (SEG_KDATA << 3);
    // tss.iomap_offset = 0xffff;
    active_proc = p;
    ltr(SEG_TSS << 3);
    vmm_switch_pdir(p->pdir);
}

// TODO: add check if fd is ok. Maybe that there is no such file.
static bool _tasking_load_bin(pdirectory_t* pdir, const char* path) {
    // file_descriptor_t fd;
    // vfs_open((file_descriptor_t*)(0), path, &fd);
    // uint8_t *prog = kmalloc(fd.size);
    // vfs_read(&fd, prog, 0, fd.size);
    // vmm_copy_to_pdir(pdir, prog, 0, fd.size);
    return true;
}

// TODO add ELF support
static bool _tasking_load(pdirectory_t* pdir, const char* path) {
    return _tasking_load_bin(pdir, path);
}


// used to jump to trapend
// the jump will start the process
void _tasking_jumper() {
    return;
}

proc_t* tasking_get_active_proc() {
    return active_proc;
}

static void _tasking_allocate_proc(proc_t *p) {
    p->pid = ++nxtpid;
    // allocating kernel stack
    p->kstack = kmalloc(VMM_PAGE_SIZE);
    char *sp = p->kstack + VMM_PAGE_SIZE;
    sp -= sizeof(*p->tf);
    p->tf = (trapframe_t *)sp;
    sp -= 4;
    *(uint32_t*)sp = (uint32_t)trap_return;
    sp -= sizeof(*p->context);
    p->context = (context_t*)sp;
    memset((void*)p->context, 0, sizeof(*p->context));
    p->context->eip = (uint32_t)_tasking_jumper;
    memset((void*)p->tf, 0, sizeof(*p->tf));
}

// Start init proccess
// All others processes will fork
void tasking_start_init_proc() {
    nxt_proc = 0;
    proc_t *p = &proc[nxt_proc++];

    // creating new page dir
    p->pdir = vmm_new_user_pdir();
    _tasking_load(p->pdir, "init.sys");
    _tasking_allocate_proc(p);
    p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
    p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
    p->tf->es = p->tf->ds;
    p->tf->ss = p->tf->ds;
    p->tf->eflags = FL_IF;
    p->tf->ebp = VMM_PAGE_SIZE;
    p->tf->esp = VMM_PAGE_SIZE;
    p->tf->eip = 0;

    // stub context
    context_t stub_cntx;
    context_t *stub_cntx_ptr = &stub_cntx;

    switchuvm(p);
    switch_contexts(&stub_cntx_ptr, p->context);
}

void tasking_fork() {
    proc_t *new_p = &proc[nxt_proc++];
    new_p->pdir = vmm_new_forked_user_pdir();
    _tasking_allocate_proc(new_p);
    memcpy((void*)new_p->tf, (void*)active_proc->tf, sizeof(trapframe_t));
    new_p->tf->eax = 0;
    active_proc->tf->eax = new_p->pid;
    switchuvm(new_p);
    switch_contexts(&active_proc->context, new_p->context);
}

// TODO add POSIX support
void tasking_exec() {
    proc_t *proc = tasking_get_active_proc();
    char *launch_path = (char*)proc->tf->ecx; // for now let's think that our string is at ecx
    vmm_zero_user_pages(proc->pdir);
    if (!_tasking_load(proc->pdir, launch_path)) {
        proc->tf->eax = -1;
        return;
    }
    proc->tf->ebp = VMM_PAGE_SIZE;
    proc->tf->esp = VMM_PAGE_SIZE;
    proc->tf->eip = 0;
}