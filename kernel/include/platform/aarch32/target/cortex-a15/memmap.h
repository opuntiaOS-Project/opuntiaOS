#ifndef __oneOS__CORTEXA15_MEMMAP_H
#define __oneOS__CORTEXA15_MEMMAP_H

#define KERNEL_PM_BASE 0x80100000
#define KERNEL_BASE 0xc0000000
#define KMALLOC_BASE (KERNEL_BASE + 0x400000)

extern struct memory_map* arm_memmap;

#endif /* __oneOS__CORTEXA15_MEMMAP_H */