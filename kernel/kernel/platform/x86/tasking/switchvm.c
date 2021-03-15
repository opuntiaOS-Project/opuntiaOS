#include <drivers/x86/fpu.h>
#include <mem/vmm/vmm.h>
#include <platform/generic/system.h>
#include <platform/x86/gdt.h>
#include <platform/x86/tasking/switchvm.h>
#include <platform/x86/tasking/tss.h>

/* switching the page dir and tss to the current proc */
void switchuvm(thread_t* thread)
{
    system_disable_interrupts();
    if (RUNNIG_THREAD) {
        fpu_save(RUNNIG_THREAD->fpu_state);
    }
    gdt[SEG_TSS] = SEG_BG(SEGTSS_TYPE, &tss, sizeof(tss) - 1, 0);
    uint32_t esp0 = ((uint32_t)thread->tf + sizeof(trapframe_t));
    tss.esp0 = esp0;
    tss.ss0 = (SEG_KDATA << 3);
    // tss.iomap_offset = 0xffff;
    RUNNIG_THREAD = thread;
    fpu_restore(thread->fpu_state);
    ltr(SEG_TSS << 3);
    vmm_switch_pdir(thread->process->pdir);
    system_enable_interrupts();
}