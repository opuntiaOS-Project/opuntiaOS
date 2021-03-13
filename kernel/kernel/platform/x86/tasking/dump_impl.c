#include <libkern/log.h>
#include <mem/vmm/vmm.h>
#include <platform/generic/system.h>
#include <platform/x86/tasking/dump_impl.h>

void dump_regs(dump_data_t* dump_data)
{
    // 3 rows of: 3chars (reg name) + 2chars (separartor) + 12chars (reg value)
    char buf[64];
    trapframe_t* tf = dump_data->p->main_thread->tf;
    snprintf(buf, 64, "EAX: %x  EBX: %x  ECX: %x\n", tf->eax, tf->ebx, tf->ecx);
    dump_data->writer(buf);
    snprintf(buf, 64, "EDX: %x  ESI: %x  EDI: %x\n", tf->edx, tf->esi, tf->edi);
    dump_data->writer(buf);
    snprintf(buf, 64, "EIP: %x  ESP: %x  EBP: %x\n\n", tf->eip, tf->esp, tf->ebp);
    dump_data->writer(buf);
}

void dump_backtrace(dump_data_t* dump_data)
{
    char buf[64];

    uint32_t id = 1;
    uint32_t ip = get_instruction_pointer(dump_data->p->main_thread->tf);
    uint32_t* bp = (uint32_t*)get_base_pointer(dump_data->p->main_thread->tf);

    do {
        if (vmm_is_kernel_address(ip)) {
            return;
        }

        snprintf(buf, 64, "[%d] %x : ", id, ip);
        dump_data->writer(buf);
        int index = dump_data->sym_resolver(dump_data->syms, dump_data->symsn, ip);
        dump_data->writer(&dump_data->strs[index]);
        dump_data->writer("\n");

        if (vmm_is_kernel_address((uint32_t)bp)) {
            return;
        }

        ip = bp[1];
        bp = (uint32_t*)*bp;
        id++;
    } while (ip != dump_data->entry_point);

    return;
}

int dump_impl(dump_data_t* dump_data)
{
    dump_regs(dump_data);
    dump_backtrace(dump_data);
    return 0;
}
