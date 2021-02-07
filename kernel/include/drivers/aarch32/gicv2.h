/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#ifndef __oneOS__DRIVERS__GICv2_H
#define __oneOS__DRIVERS__GICv2_H

#include <drivers/driver_manager.h>
#include <platform/aarch32/interrupts.h>
#include <platform/aarch32/target/cortex-a15/device_settings.h>
#include <types.h>
#include <utils/mask.h>

enum GICDControlMasks {
    MASKDEFINE(GICD_ENABLE, 0, 1),
};

enum GICCControlMasks {
    MASKDEFINE(GICC_ENABLE_GR1, 0, 1),
    MASKDEFINE(GICC_FIQ_BYP_DIS_GR1, 5, 1),
    MASKDEFINE(GICC_IRQ_BYP_DIS_GR1, 6, 1),
    MASKDEFINE(GICC_EO_IMODE_NS, 9, 1),
};

struct gicv2_distributor_registers {
    uint32_t control;
    uint32_t typer;
    uint32_t iidr;
    SKIP(0x008 + 0x4, 0x080);
    uint32_t igroup[8];
    SKIP(0x09C + 0x4, 0x100);
    uint32_t isenabler[8];
    SKIP(0x11C + 0x4, 0x180);
    uint32_t icenabler[8];
    SKIP(0x19C + 0x4, 0x200);
    uint32_t ispendr[8];
    SKIP(0x21C + 0x4, 0x280);
    uint32_t icpendr[8];
    SKIP(0x29C + 0x4, 0x300);
    uint32_t isactiver[8];
    SKIP(0x31C + 0x4, 0x380);
    uint32_t icactiver[8];
    SKIP(0x39C + 0x4, 0x400);
    uint32_t ipriorityr[64];
    SKIP(0x4FC + 0x4, 0x800);
    uint32_t itargetsr[64];
    SKIP(0x8FC + 0x4, 0xC00);
    uint32_t icfgr[16];
    // TO BE CONTINUED
};
typedef struct gicv2_distributor_registers gicv2_distributor_registers_t;

struct gicv2_cpu_interface_registers {
    uint32_t control;
    uint32_t pmr;
    uint32_t bpr;
    uint32_t iar;
    uint32_t eoir;
    uint32_t hppir;
    uint32_t abpr;
    uint32_t aiar;
    uint32_t aeoir;
    uint32_t ahppir;
    // TO BE CONTINUED
};
typedef struct gicv2_cpu_interface_registers gicv2_cpu_interface_registers_t;

void gicv2_enable_irq(irq_line_t id, irq_priority_t prior, irq_type_t type);
void gicv2_install();
uint32_t gicv2_interrupt_descriptor();
void gicv2_end(uint32_t int_disc);

#endif //__oneOS__DRIVERS__GICv2_H
