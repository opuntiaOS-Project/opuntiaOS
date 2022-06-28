/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/irq/x86/pic.h>

void pic_remap(unsigned int offset1, unsigned int offset2)
{
    unsigned char m1, m2;
    m1 = port_read8(MASTER_PIC_DATA);
    m2 = port_read8(SLAVE_PIC_DATA);

    port_write8(MASTER_PIC_CMD, 0x11); // start in cascade mode
    port_wait_io();
    port_write8(SLAVE_PIC_CMD, 0x11); // start in cascade mode
    port_wait_io();
    port_write8(MASTER_PIC_DATA, offset1);
    port_wait_io();
    port_write8(SLAVE_PIC_DATA, offset2);
    port_wait_io();
    port_write8(MASTER_PIC_DATA, 0x04); // 0000 0100 - use irq2 in cascade
    port_wait_io();
    port_write8(SLAVE_PIC_DATA, 0x02); // cascade identity
    port_wait_io();

    port_write8(MASTER_PIC_DATA, ICW4_8086);
    port_wait_io();
    port_write8(SLAVE_PIC_DATA, ICW4_8086);
    port_wait_io();
    port_write8(MASTER_PIC_DATA, 0x00);
    port_write8(SLAVE_PIC_DATA, 0x00);
}