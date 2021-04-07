/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/x86/rtc.h>
#include <platform/x86/port.h>

#define BCD_TO_BIN(v) ((((v)&0xF0) >> 1) + (((v)&0xF0) >> 3) + ((v)&0xf))

inline static uint8_t cmos_read(uint8_t port)
{
    port_8bit_out(0x70, port);
    return port_8bit_in(0x71);
}

inline static void cmos_wait_while_updating()
{
    while ((cmos_read(0x0A) & 0x80)) { }
}

void rtc_load_time(uint8_t* secs, uint8_t* mins, uint8_t* hrs, uint8_t* day, uint8_t* month, uint32_t* year)
{
    cmos_wait_while_updating();
    uint8_t status_reg_b = cmos_read(0x0b);
    *secs = cmos_read(0x00);
    *mins = cmos_read(0x02);
    *hrs = cmos_read(0x04);
    *day = cmos_read(0x07);
    *month = cmos_read(0x08);
    *year = cmos_read(0x09);

    if (!(status_reg_b & 0b100)) {
        *secs = BCD_TO_BIN(*secs);
        *mins = BCD_TO_BIN(*mins);
        *hrs = BCD_TO_BIN(*hrs);
        *day = BCD_TO_BIN(*day);
        *month = BCD_TO_BIN(*month);
        *year = BCD_TO_BIN(*year);
    }

    *year += 2000;

    if (((*hrs) >> 8) & 1) {
        *hrs = ((*hrs & 0x7F) + 12) % 24;
    }
}