/**
 * Stage2 is used to load main kernel.
 */

#include "types.h"
#include "config.h"
#include "mem/vm.h"
#include "mem/mem_map.h"
#include "drivers/ata.h"
#include "drivers/ext2_lite.h"
#include "drivers/drive_desc.h"
#include "drivers/fs_desc.h"
#include "drivers/display.h"

int get_load_disk(drive_desc_t *drive_desc) {
    init_ata(0x1F0, 1);
    if (indentify_ata_device(drive_desc) == 0) {
        printh((uint32_t)drive_desc->read);
        return 0;
    }
    return -1;
}

int get_fs_of_disk(drive_desc_t *drive_desc, fs_desc_t *fs_desc) {
    if (ext2_lite_init(drive_desc, fs_desc) == 0) {
        return 0;
    }
    return -1;
}

void stage2(mem_desc_t *mem_desc) {
    clean_screen();
    printf("STAGE2\n");
    drive_desc_t drive_desc;
    fs_desc_t fs_desc;

    printf("P1");

    if (get_load_disk(&drive_desc) != 0) {
        printf("E1");
        while(1) {}
    }

    printf("P2");

    if (get_fs_of_disk(&drive_desc, &fs_desc) != 0) {
        printf("E2");
        while(1) {}
    }

    printf("P3");

    // printh((uint32_t)drive_desc.read);
    // printh((uint32_t)fs_desc.read);
    int (*read_kernel)(drive_desc_t *drive_desc, char *path, uint8_t *read_to) = fs_desc.read;
    read_kernel(&drive_desc, KERNEL_PATH, (uint8_t*)KERNEL_BASE);

    // uint32_t place_to = 0x100000;
    // mem_desc->kernel_size = ata_read_to_ram(&ata0m, 1, place_to, 2);
    // place_to += 512 - 2;
    // for (uint16_t i = 1; i < mem_desc->kernel_size * 2; i++) {
    //     ata_read_to_ram(&ata0m, i+1, place_to, 0);
    //     place_to += 512;
    // }

    vm_setup();

    // enabling paging
    asm volatile ("mov %cr0, %eax");
    asm volatile ("or $0x80000000, %eax");
    asm volatile ("mov %eax, %cr0");

    asm volatile ("add $0xffc00000, %ebp");
    asm volatile ("add $0xffc00000, %esp");

    asm volatile("push %0" : : "r"(mem_desc));
    asm volatile("mov $0xc0000000, %eax");
    asm volatile("call %eax");
    while (1) {}
}
