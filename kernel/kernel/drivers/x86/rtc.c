/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/driver_manager.h>
#include <drivers/x86/rtc.h>
#include <platform/x86/port.h>
#include <time/time_manager.h>

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

uint32_t rtc_load_time()
{
    cmos_wait_while_updating();
    uint8_t status_reg_b = cmos_read(0x0b);
    uint8_t secs = cmos_read(0x00);
    uint8_t mins = cmos_read(0x02);
    uint8_t hrs = cmos_read(0x04);
    uint8_t day = cmos_read(0x07);
    uint8_t month = cmos_read(0x08);
    uint32_t year = cmos_read(0x09);

    if (!(status_reg_b & 0b100)) {
        secs = BCD_TO_BIN(secs);
        mins = BCD_TO_BIN(mins);
        hrs = BCD_TO_BIN(hrs);
        day = BCD_TO_BIN(day);
        month = BCD_TO_BIN(month);
        year = BCD_TO_BIN(year);
    }

    year += 2000;

    if (((hrs) >> 8) & 1) {
        hrs = ((hrs & 0x7F) + 12) % 24;
    }

    return timeman_to_seconds_since_epoch(secs, mins, hrs, day, month, year);
}

int rtc_init(device_t* dev)
{
    if (dev->device_desc.type != DEVICE_DESC_DEVTREE) {
        return -1;
    }
    return 0;
}

static driver_desc_t _rtc_driver_info()
{
    driver_desc_t rtc_desc = { 0 };
    rtc_desc.type = DRIVER_RTC;
    rtc_desc.system_funcs.init_with_dev = rtc_init;
    rtc_desc.functions[DRIVER_RTC_GET_TIME] = rtc_load_time;
    return rtc_desc;
}

static device_desc_t _rtc_device_info()
{
    devtree_entry_t entry = {
        .region_base = 0x0,
        .region_size = 0x0,
        .type = DEVTREE_ENTRY_TYPE_RTC,
        .rel_name_offset = devtree_new_entry_name("rtc"),
        .flags = 0x0,
    };

    device_desc_t new_device = { 0 };
    new_device.type = DEVICE_DESC_DEVTREE;
    new_device.devtree.entry = devtree_new_entry(&entry);
    return new_device;
}

void rtc_install()
{
    devman_register_driver(_rtc_driver_info(), "rtc");
    devman_register_device(_rtc_device_info(), DEVICE_RTC);
}
devman_register_driver_installation(rtc_install);