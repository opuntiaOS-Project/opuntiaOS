#include <utils/kernel_self_test.h>
#include <drivers/display.h>
#include <mem/malloc.h>
#include <mem/vmm/vmm.h>

bool _test_kmalloc();
bool _test_page_fault();

bool _test_kmalloc() {
    uint32_t* kek1 = (uint32_t*)kmalloc(sizeof(uint32_t));
    uint32_t* kek2 = (uint32_t*)kmalloc(sizeof(uint32_t));
    *kek1 = 1;
    *kek2 = 2;
    return *kek1 == 1 && *kek2 == 2;
}

bool _test_page_fault() {
    int* newpage = (int *)0x10000000;
    *newpage = 8;
    return *newpage == 8;
}

void kpanic(char *t_err_msg) {
    clean_screen();
    printf("*****\n");
    printf("Kernel Panic\n");
    printf("*****\n");
    printf(t_err_msg);
    printf("\n*****");
    while (1) {}
}

void kpanic_at_test(char *t_err_msg, uint16_t test_no) {
    clean_screen();
    printf("*****\n");
    printf("Kernel Panic\n");
    printf("*****\n");
    printf(t_err_msg);
    printf("\nFailed Test: "); printd(test_no);
    printf("\n*****");
    while (1) {}
}

bool kernel_self_test(bool throw_kernel_panic) {
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
                while (1) {}
                kpanic_at_test("Kernel-self-test NOT passed", i);
            } else {
                return false;
            }
        }
        i++;
    }
    return true;
}

// Archieved
// bool load_app(ata_t* ata0m) {
//     uint8_t* new_block = pmm_alloc_block();
//     vmm_map_page(new_block, 0x60000000);
//     uint8_t *app = (uint8_t *)0x60000000;
//     printh(app);
//     uint8_t* read_buffer = (uint8_t*)kmalloc(512);
//     ata_read(ata0m, 60, read_buffer);

//     for (int i = 0; i < 256; i+=2) {
//         app[i] = read_buffer[i+1];
//         app[i+1] = read_buffer[i];
//         printh(app[i]); printf(" ");
//         printh(app[i+1]); printf(" ");
//     }

//     printf("\n\nTests passed [ENTER to continue]");

//     uint32_t key = KEY_UNKNOWN;
//     while (key != KEY_RETURN) {
// 		key = kbdriver_get_last_key();
//     }
//     kbdriver_discard_last_key();

//     asm volatile("mov $0x60000000, %eax");
//     asm volatile("call %eax"); // test interrupts
//     uint32_t return_value;
//     __asm__("mov %%eax, %%eax" : "=a" (return_value) :);

//     printd(return_value);

//     printf("\n\nTests passed [ENTER to continue]");

//     key = KEY_UNKNOWN;
//     while (key != KEY_RETURN) {
// 		key = kbdriver_get_last_key();
//     }
//     kbdriver_discard_last_key();
//     // while(1) {}
// }