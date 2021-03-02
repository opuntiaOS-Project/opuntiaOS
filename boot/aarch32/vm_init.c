typedef unsigned long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;
typedef long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;
typedef char bool;

#define VMM_PTE_COUNT (256)
#define VMM_PDE_COUNT (4096)
#define VMM_PAGE_SIZE (4096)

struct table_desc {
    int valid : 1; /* Valid mapping */
    int zero1 : 1;
    int zero2 : 1;
    int ns : 1;
    int zero3 : 1;
    int domain : 4;
    int imp : 1;
    int baddr : 22;
} __attribute__((packed));
typedef struct table_desc table_desc_t;

struct page_desc {
    int xn : 1; // Execute never. Stops execution of page.
    int one : 1; // Always one for tables
    int b : 1; // cacheable
    int c : 1; // Cacheable
    int ap1 : 2;
    int tex : 3;
    int ap2 : 1;
    int s : 1;
    int ng : 1;
    int baddr : 20;
} __attribute__((packed));
typedef struct page_desc page_desc_t;

typedef struct {
    page_desc_t entities[VMM_PTE_COUNT];
} ptable_t;

typedef struct pdirectory {
    table_desc_t entities[VMM_PDE_COUNT];
} pdirectory_t;

void vm_setup() __attribute__((section(".init_code")));
static ptable_t* map_page() __attribute__((section(".init_code")));
static void write_ttbcr() __attribute__((section(".init_code")));
static void write_ttbr0() __attribute__((section(".init_code")));
static void mmu_enable() __attribute__((section(".init_code")));
static void write_dacr() __attribute__((section(".init_code")));
static pdirectory_t* __attribute__((section(".init_data"))) dir = (pdirectory_t*)0x80000000;
static ptable_t* __attribute__((section(".init_data"))) next_table = (ptable_t*)0x80004000;

#define VMM_OFFSET_IN_DIRECTORY(a) (((a) >> 20) & 0xfff)
#define VMM_OFFSET_IN_TABLE(a) (((a) >> 12) & 0xff)
#define VMM_OFFSET_IN_PAGE(a) ((a)&0xfff)

static inline void write_ttbcr(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c2, c0, 2"
                 :
                 : "r"(val)
                 : "memory");
}

static inline void write_ttbr0(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c2, c0, 0"
                 :
                 : "r"(val)
                 : "memory");
    asm volatile("dmb");
}

static inline void mmu_enable()
{
    volatile uint32_t val;
    asm volatile("mrc p15, 0, %0, c1, c0, 0"
                 : "=r"(val));
    asm volatile("orr %0, %1, #0x1"
                 : "=r"(val)
                 : "r"(val));
    asm volatile("mcr p15, 0, %0, c1, c0, 0" ::"r"(val)
                 : "memory");
    asm volatile("isb");
}

static inline void write_dacr(uint32_t val)
{
    asm volatile("mcr p15, 0, %0, c3, c0, 0"
                 :
                 : "r"(val));
}

static ptable_t* map_page(uint32_t tphyz, uint32_t tvirt)
{
    ptable_t* table = next_table;
    for (uint32_t phyz = tphyz, virt = tvirt, i = 0; i < VMM_PTE_COUNT; phyz += VMM_PAGE_SIZE, virt += VMM_PAGE_SIZE, i++) {
        page_desc_t new_page;
        new_page.one = 1;
        new_page.baddr = (phyz / VMM_PAGE_SIZE);
        new_page.tex = 0b001;
        new_page.c = 1;
        new_page.b = 1;
        new_page.ap1 = 0b11;
        new_page.ap2 = 0b0;
        new_page.s = 1;
        table->entities[i] = new_page;
    }

    uint32_t table_int = (uint32_t)table;
    dir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].valid = 1;
    dir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].zero1 = 0;
    dir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].zero2 = 0;
    dir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].ns = 0;
    dir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].zero3 = 0;
    dir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].domain = 0b0011;
    dir->entities[VMM_OFFSET_IN_DIRECTORY(tvirt)].baddr = ((table_int / 1024));

    next_table++;
    return table;
}

void vm_setup()
{
    for (int i = 0; i < VMM_PDE_COUNT; i++) {
        dir->entities[i].valid = 0;
    }
    
    map_page(0x1c000000, 0x1c000000); // mapping uart
    map_page(0x3f000000, 0x3f000000); // mapping interrupt controller
    map_page(0x80100000, 0xc0000000); // kernel
    map_page(0x80200000, 0xc0100000); // kernel
    map_page(0x80300000, 0xc0200000); // kernel
    map_page(0x80400000, 0xc0300000); // kernel
    map_page(0x80000000, 0x80000000); // kernel to self
    map_page(0x80100000, 0x80100000); // kernel to self
    map_page(0x80200000, 0x80200000); // kernel to self
    map_page(0x80300000, 0x80300000); // kernel to self

    write_ttbr0((uint32_t)(0x80000000));
    write_dacr(0x55555555);
    mmu_enable();
}