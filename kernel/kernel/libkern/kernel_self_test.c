#ifdef __i386__

#include <drivers/x86/display.h>
#include <libkern/kernel_self_test.h>
#include <mem/kmalloc.h>
#include <mem/vmm/vmm.h>

bool _test_kmalloc();
bool _test_page_fault();

bool _test_kmalloc()
{
    uint32_t* kek1 = (uint32_t*)kmalloc(sizeof(uint32_t));
    uint32_t* kek2 = (uint32_t*)kmalloc(sizeof(uint32_t));
    *kek1 = 1;
    *kek2 = 2;
    return *kek1 == 1 && *kek2 == 2;
}

bool _test_page_fault()
{
    int* newpage = (int*)0x10000000;
    *newpage = 8;
    return *newpage == 8;
}

void kpanic_at_test(char* t_err_msg, uint16_t test_no)
{
    while (1) { }
}

bool kernel_self_test(bool throw_kernel_panic)
{
    void* active_test[] = {
        _test_kmalloc,
        _test_page_fault,
        0 // end sign
    };

    uint16_t i = 0;
    while (active_test[i] != 0) {
        bool (*test)() = active_test[i];
        if (!test()) {
            if (throw_kernel_panic) {
                while (1) { }
                kpanic_at_test("Kernel-self-test NOT passed", i);
            } else {
                return false;
            }
        }
        i++;
    }
    return true;
}

#endif