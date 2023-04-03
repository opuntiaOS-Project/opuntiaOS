/*
 * Copyright (C) 2020-2023 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/irq/irq_api.h>
#include <drivers/irq/riscv/plic.h>
#include <libkern/libkern.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>
#include <platform/generic/registers.h>

#define DEBUG_PLIC

static irqdev_descritptor_t plic_descriptor = {
    .interrupt_descriptor = plic_interrupt_descriptor,
    .end_interrupt = plic_end,
    .enable_irq = plic_enable_irq,
};

struct plic_mappings {
    char* plic;
    char* plic_ext;
    kmemzone_t plic_zone;
    kmemzone_t plic_zone_ext;
};
typedef struct plic_mappings plic_mappings_t;
plic_mappings_t mappings;

static inline uint32_t* _plic_priorities(irq_line_t id) { return (uint32_t*)(mappings.plic + id * 4); }
static inline uint32_t* _plic_hart_enable(int hart) { return (uint32_t*)(mappings.plic + 0x2080 + hart * 0x100); }
static inline uint32_t* _plic_hart_priority(int hart) { return (uint32_t*)(mappings.plic_ext + hart * 0x2000); }
static inline uint32_t* _plic_hart_claim(int hart) { return (uint32_t*)(mappings.plic_ext + 0x4 + hart * 0x2000); }

static inline int _plic_map_itself(int hart_id)
{
    ASSERT(hart_id == 0 && "Mapping scale too bad for several harts");
    devtree_entry_t* device = devtree_find_device("plic");
    if (!device) {
        kpanic("PLIC: Can't find device in the tree.");
    }

    uintptr_t base = device->region_base;

    size_t plic_zone_pages = 4;
    mappings.plic_zone = kmemzone_new(VMM_PAGE_SIZE * plic_zone_pages);
    vmm_map_pages(mappings.plic_zone.start, base, plic_zone_pages, MMU_FLAG_DEVICE);
    mappings.plic = (char*)mappings.plic_zone.ptr;

    size_t plic_zone_ext_pages = 1;
    mappings.plic_zone_ext = kmemzone_new(VMM_PAGE_SIZE * plic_zone_ext_pages);
    vmm_map_pages(mappings.plic_zone_ext.start, base + 0x201000, plic_zone_ext_pages, MMU_FLAG_DEVICE);
    mappings.plic_ext = (char*)mappings.plic_zone_ext.ptr;
    return 0;
}

void plic_install()
{
    if (_plic_map_itself(0)) {
#ifdef DEBUG_PLIC
        log_error("PLIC: Can't map itself!");
#endif
        return;
    }

    irq_set_dev(plic_descriptor);

    volatile uint32_t* hart_priority = _plic_hart_priority(0);
    *hart_priority = 0;
}

void plic_enable_irq(irq_line_t id, irq_priority_t prior, irq_flags_t flags, int cpu_mask)
{
    // Defualt minimum priority inside OS is 0, but 0 for plic means interrupt is disabled, do add 1.
    prior++;
    volatile uint32_t* prior_ptr = _plic_priorities(id);
    *prior_ptr = prior;

    volatile uint32_t* hart_enabled = _plic_hart_enable(0);
    *hart_enabled = *hart_enabled | (1 << id);
}

uint32_t plic_interrupt_descriptor()
{
    volatile uint32_t* hart_claim = _plic_hart_claim(0);
    return *hart_claim;
}

void plic_end(uint32_t id)
{
    volatile uint32_t* hart_claim = _plic_hart_claim(0);
    *hart_claim = id;
}