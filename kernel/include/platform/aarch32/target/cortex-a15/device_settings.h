/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_PLATFORM_AARCH32_TARGET_CORTEX_A15_DEVICE_SETTINGS_H
#define _KERNEL_PLATFORM_AARCH32_TARGET_CORTEX_A15_DEVICE_SETTINGS_H

/**
 * Used devices:
 *      uart
 *      gicv2
 *      sp804
 *      pl181
 *      pl111
 *      pl050
 *      pl031
 */

/* Base is read from CBAR */
#define GICv2_DISTRIBUTOR_OFFSET 0x1000
#define GICv2_CPU_INTERFACE_OFFSET 0x2000

#define UART_BASE 0x1c090000

#define SP804_BASE 0x1c110000

#define PL181_BASE 0x1c050000

#define PL111_BASE 0x1c1f0000

#define PL050_KEYBOARD_BASE 0x1c060000

#define PL050_MOUSE_BASE 0x1c070000

#define PL031_BASE 0x1c170000

/**
 * Interrupt lines:
 *      SP804 TIMER1: 2nd line in SPI (32+2)
 */

#define SP804_TIMER1_IRQ_LINE (32 + 2)

#define PL050_KEYBOARD_IRQ_LINE (32 + 12)
#define PL050_MOUSE_IRQ_LINE (32 + 13)

#endif /* _KERNEL_PLATFORM_AARCH32_TARGET_CORTEX_A15_DEVICE_SETTINGS_H */