/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/gicv2.h>
#include <libkern/log.h>
#include <mem/vmm/vmm.h>
#include <mem/vmm/zoner.h>
#include <platform/aarch32/interrupts.h>
#include <platform/aarch32/registers.h>

// #define DEBUG_GICv2
#define IS_SGI(id) ((id) < 16)
#define IS_PPI(id) ((id) < 32 && (id) >= 16)
#define IS_SGI_OR_PPI(id) ((id) < 32)
#define IS_SPI(id) ((id) < 1020 && (id) >= 32)

static gic_descritptor_t gicv2_descriptor = {
    .interrupt_descriptor = gicv2_interrupt_descriptor,
    .end_interrupt = gicv2_end,
    .enable_irq = gicv2_enable_irq,
};
static zone_t distributor_zone;
static zone_t cpu_interface_zone;
volatile gicv2_distributor_registers_t* distributor_registers;
volatile gicv2_cpu_interface_registers_t* cpu_interface_registers;

static inline int _gicv2_map_itself()
{
    uint32_t cbar = read_cbar();

    distributor_zone = zoner_new_zone(VMM_PAGE_SIZE);
    vmm_map_page(distributor_zone.start, cbar + GICv2_DISTRIBUTOR_OFFSET, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE | PAGE_NOT_CACHEABLE);
    distributor_registers = (gicv2_distributor_registers_t*)distributor_zone.ptr;

    cpu_interface_zone = zoner_new_zone(VMM_PAGE_SIZE);
    vmm_map_page(cpu_interface_zone.start, cbar + GICv2_CPU_INTERFACE_OFFSET, PAGE_READABLE | PAGE_WRITABLE | PAGE_EXECUTABLE | PAGE_NOT_CACHEABLE);
    cpu_interface_registers = (gicv2_cpu_interface_registers_t*)cpu_interface_zone.ptr;
    return 0;
}

void gicv2_enable_irq(irq_line_t id, irq_priority_t prior, irq_type_t type, int cpu_mask)
{
    int id_1bit_offset = id / 32;
    int id_1bit_bitpos = id % 32;
    int id_2bit_offset = id / 16;
    int id_2bit_bitpos = (id % 16) * 2;
    int id_8bit_offset = id / 4;
    int id_8bit_bitpos = (id % 4) * 8;

    /* We don't turn on group 1 for SPI, but do for SGI and PPI */
    if (IS_SGI_OR_PPI(id)) {
        distributor_registers->igroup[id_1bit_offset] |= (1 << id_1bit_bitpos);
    }

    /* Setting priority */
    distributor_registers->ipriorityr[id_8bit_offset] |= (prior << id_8bit_bitpos);

    if ((type & IRQ_TYPE_EDGE_TRIGGERED_MASK) == IRQ_TYPE_EDGE_TRIGGERED_MASK) {
        distributor_registers->ipriorityr[id_8bit_offset] |= (prior << id_8bit_bitpos);
    } else {
        distributor_registers->ipriorityr[id_8bit_offset] &= ~(uint32_t)(prior << id_8bit_bitpos);
    }

    distributor_registers->icfgr[id_2bit_offset] |= (0b11 << id_2bit_bitpos);

    if (IS_SPI(id)) {
        distributor_registers->itargetsr[id_8bit_offset] |= (cpu_mask << id_8bit_bitpos);
    }

    /* Enabling */
    distributor_registers->isenabler[id_1bit_offset] |= (1 << id_1bit_bitpos);
}

void gicv2_install()
{
    if (_gicv2_map_itself()) {
#ifdef DEBUG_GICv2
        log_error("GICv2: Can't map itself!");
#endif
        return;
    }

    irq_set_gic_desc(gicv2_descriptor);

#ifdef DEBUG_GICv2
    log("Gic type %x", distributor_registers->typer);
#endif

    distributor_registers->control = GICD_ENABLE_MASK;
    cpu_interface_registers->pmr = 0xff;
    cpu_interface_registers->bpr = 0x0;
    cpu_interface_registers->control = GICC_ENABLE_GR1_MASK;
}

void gicv2_install_secondary_cpu()
{
#ifdef DEBUG_GICv2
    log("Gic type %x", distributor_registers->typer);
#endif
    distributor_registers->control = GICD_ENABLE_MASK;
    cpu_interface_registers->pmr = 0xff;
    cpu_interface_registers->bpr = 0x0;
    cpu_interface_registers->control = GICC_ENABLE_GR1_MASK;
}

uint32_t gicv2_interrupt_descriptor()
{
    return cpu_interface_registers->iar;
}

void gicv2_end(uint32_t int_disc)
{
    cpu_interface_registers->eoir = int_disc;
}