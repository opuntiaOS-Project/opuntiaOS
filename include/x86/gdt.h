#ifndef __oneOS__X86__GDT_H
#define __oneOS__X86__GDT_H

#include <types.h>

#define GDT_MAX_ENTRIES 6
#define SEG_KCODE 1  // kernel code
#define SEG_KDATA 2  // kernel data+stack
#define SEG_UCODE 3  // user code
#define SEG_UDATA 4  // user data+stack
#define SEG_TSS   5  // task state NOT USED CURRENTLY 

#define SEGF_X 0x8 // exec
#define SEGF_A 0x1 // accessed 
#define SEGF_R 0x2 // readable (if exec)
#define SEGF_C 0x4 // conforming seg (if exec) 
#define SEGF_W 0x2 // writeable (if non-exec)
#define SEGF_D 0x4 // grows down (if non-exec)

#define FL_IF 0x00000200

#define DPL_KERN 0x0
#define DPL_USER 0x3

struct gdt_entry {
    uint32_t lim_15_0 : 16;
    uint32_t base_15_0 : 16;
    uint32_t base_23_16 : 8;
    uint32_t type : 4;
    uint32_t dt : 1;
    uint32_t dpl : 2;
    uint32_t p : 1;
    uint32_t lim_19_16 : 4;
    uint32_t avl : 1;
    uint32_t rsv1 : 1;
    uint32_t db : 1;
    uint32_t g : 1;
    uint32_t base_31_24 : 8;
} __attribute__((packed));

extern struct gdt_entry gdt[GDT_MAX_ENTRIES];

// segment with page granularity
#define SEG_PG(type, base, limit, dpl) (struct gdt_entry)         \
{ ((limit) >> 12) & 0xffff, (uint32_t)(base) & 0xffff,         \
  ((uint32_t)(base) >> 16) & 0xff, type, 1, dpl, 1,            \
  ((uint32_t)(limit) >> 28), 0, 0, 1, 1, (uint32_t)(base) >> 24  \
}

// segment with byte granularity
#define SEG_BG(type, base, limit, dpl) (struct gdt_entry)         \
{ ((limit)) & 0xffff, (uint32_t)(base) & 0xffff,         \
  ((uint32_t)(base) >> 16) & 0xff, type, 0, dpl, 1,            \
  (((uint32_t)(limit) >> 16) & 0xf), 0, 0, 0, 0, (uint32_t)(base) >> 24  \
}

void gdt_setup();

#endif // __oneOS__X86__GDT_H