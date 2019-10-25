#include <types.h>
#include <drivers/ata.h>

typedef struct {
    uint16_t memory_map_size;
    uint16_t kernel_size;
} mem_desc_t;

void stage2(mem_desc_t *mem_desc) {
    ata_t ata0m;
    init_ata(&ata0m, 0x1F0, 1);
    indentify_ata_device(&ata0m);
    uint32_t place_to = 0x100000;
    mem_desc->kernel_size = ata_read_to_ram(&ata0m, 0, place_to, 2);
    place_to += 512 - 2;
    for (uint16_t i = 1; i < mem_desc->kernel_size * 2; i++) {
        ata_read_to_ram(&ata0m, i, place_to, 0);
        place_to += 512;
    }
    uint8_t *ram = (uint8_t *)0x100000;
    asm volatile("push %0" : : "r"(mem_desc));
    asm volatile("mov $0x100000, %eax");
    asm volatile("call %eax");
    while (1) {}
}