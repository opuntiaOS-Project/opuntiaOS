#include <mem/vmm/vmm.h>
#include <platform/aarch32/interrupts.h>
#include <platform/generic/system.h>
#include <platform/generic/tasking/trapframe.h>
#include <tasking/tasking.h>

/* switching the page dir and tss to the current proc */
void switchuvm(thread_t* thread)
{
    system_disable_interrupts();
    // uint32_t esp0 = ((uint32_t)thread->tf + sizeof(trapframe_t));
    // set_svc_stack(esp0);
    // set_irq_stack(esp0);
    RUNNIG_THREAD = thread;
    vmm_switch_pdir(thread->process->pdir);
    system_enable_interrupts();
}