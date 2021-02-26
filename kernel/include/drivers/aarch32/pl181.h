/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _KERNEL_DRIVERS_AARCH32_PL181_H
#define _KERNEL_DRIVERS_AARCH32_PL181_H

#include <drivers/driver_manager.h>
#include <platform/aarch32/target/cortex-a15/device_settings.h>
#include <libkern/types.h>
#include <libkern/mask.h>

#define PL181_SECTOR_SIZE 512

enum PL181CommandMasks {
    MASKDEFINE(MMC_CMD_IDX, 0, 6),
    MASKDEFINE(MMC_CMD_RESP, 6, 1),
    MASKDEFINE(MMC_CMD_LONG_RESP, 7, 1),
    MASKDEFINE(MMC_CMD_INTERRUPT, 8, 1),
    MASKDEFINE(MMC_CMD_PENDING, 9, 1),
    MASKDEFINE(MMC_CMD_ENABLE, 10, 1),
};

enum PL181StatusMasks {
    MASKDEFINE(MMC_STAT_CRC_FAIL, 0, 1),
    MASKDEFINE(MMC_STAT_CMD_TIMEOUT, 2, 1),
    MASKDEFINE(MMC_STAT_CMD_RESP_END, 6, 1),
    MASKDEFINE(MMC_STAT_CMD_SENT, 7, 1),
    MASKDEFINE(MMC_STAT_CMD_ACTIVE, 11, 1),
    MASKDEFINE(MMC_STAT_TRANSMIT_FIFO_EMPTY, 18, 1),
    MASKDEFINE(MMC_STAT_FIFO_DATA_AVAIL_TO_READ, 21, 1),
};

enum PL181Commands {
    CMD_GO_IDLE_STATE = 0,
    CMD_ALL_SEND_CID = 2,
    CMD_SET_RELATIVE_ADDR = 3,
    CMD_SELECT = 7,
    CMD_SET_SECTOR_SIZE = 16,
    CMD_READ_SINGLE_BLOCK = 17,
    CMD_WRITE_SINGLE_BLOCK = 24,
    CMD_SD_SEND_OP_COND = 41,
    CMD_APP_CMD = 55,
};

struct pl181_registers {
    uint32_t power;
    uint32_t clock;
    uint32_t arg;
    uint32_t cmd;
    uint32_t resp_cmd;
    uint32_t response[4];
    uint32_t data_timer;
    uint32_t data_length;
    uint32_t data_control;
    uint32_t data_count;
    uint32_t status;
    uint32_t clear;
    uint32_t interrupt_mask[2];
    uint32_t interrupt_select;
    uint32_t fifo_count;
    char res[0x34];
    uint32_t fifo_data[16];
    // TO BE CONTINUED
};
typedef struct pl181_registers pl181_registers_t;

struct sd_card {
    uint32_t rca;
    uint32_t ishc;
};
typedef struct sd_card sd_card_t;

void pl181_install();

#endif //_KERNEL_DRIVERS_AARCH32_PL181_H
