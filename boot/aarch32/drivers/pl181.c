/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "pl181.h"
#include <libboot/devtree/devtree.h>
#include <libboot/log/log.h>

// #define DEBUG_PL181

static sd_card_t sd_card;
static volatile pl181_registers_t* registers;

static inline int _pl181_map_itself(devtree_entry_t* dev)
{
    registers = (pl181_registers_t*)dev->region_base;
    return 0;
}

static inline void _pl181_clear_flags()
{
    registers->clear = 0x5FF;
}

static int _pl181_send_cmd(uint32_t cmd, uint32_t param)
{
    _pl181_clear_flags();

    registers->arg = param;
    registers->cmd = cmd;

    while (registers->status & MMC_STAT_CMD_ACTIVE_MASK) { }

    if ((cmd & MMC_CMD_RESP_MASK) == MMC_CMD_RESP_MASK) {
        while (((registers->status & MMC_STAT_CMD_RESP_END_MASK) != MMC_STAT_CMD_RESP_END_MASK) || (registers->status & MMC_STAT_CMD_ACTIVE_MASK)) {
            if (registers->status & MMC_STAT_CMD_TIMEOUT_MASK) {
                return -1;
            }
        }
    } else {
        while ((registers->status & MMC_STAT_CMD_SENT_MASK) != MMC_STAT_CMD_SENT_MASK) { }
    }
    return 0;
}

static int _pl181_send_app_cmd(uint32_t cmd, uint32_t param)
{
    for (int i = 0; i < 5; i++) {
        _pl181_send_cmd(CMD_APP_CMD | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, 0);
        if (registers->response[0] & (1 << 5)) {
            return _pl181_send_cmd(cmd, param);
        }
    }

    log_error("Failed to send app command");
    return -4;
}

inline static int _pl181_select_card(uint32_t rca)
{
    return _pl181_send_cmd(CMD_SELECT | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, rca);
}

static int _pl181_read_block(uint32_t lba_like, void* read_data)
{
    sd_card_t* sd_card_ptr = &sd_card;
    uint32_t* read_data32 = (uint32_t*)read_data;
    uint32_t bytes_read = 0;

    registers->data_length = PL181_SECTOR_SIZE; // Set length of bytes to transfer
    registers->data_control = 0b11; // Enable dpsm and set direction from card to host

    if (sd_card_ptr->ishc) {
        _pl181_send_cmd(CMD_READ_SINGLE_BLOCK | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, lba_like);
    } else {
        _pl181_send_cmd(CMD_READ_SINGLE_BLOCK | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, lba_like * PL181_SECTOR_SIZE);
    }

    while (registers->status & MMC_STAT_FIFO_DATA_AVAIL_TO_READ_MASK) {
        *read_data32 = registers->fifo_data[0];
        read_data32++;
        bytes_read += 4;
    }
    return bytes_read;
}

static int _pl181_register_device(uint32_t rca, bool ishc, uint32_t capacity)
{
    sd_card.rca = rca;
    sd_card.ishc = ishc;
    sd_card.capacity = capacity;
    _pl181_select_card(rca);
    _pl181_send_cmd(CMD_SET_SECTOR_SIZE | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, PL181_SECTOR_SIZE);
    if (registers->response[0] != 0x900) {
        log_error("PL181(pl181_add_new_device): Can't set sector size");
        return -1;
    }

    return 0;
}

int pl181_init(drive_desc_t* drive_desc)
{
    devtree_entry_t* dev = devtree_find_device("pl181");
    if (!dev) {
        return -1;
    }

    if (_pl181_map_itself(dev)) {
#ifdef DEBUG_PL181
        log_error("PL181: Can't map itself!");
#endif
        return -1;
    }

#ifdef DEBUG_PL181
    log("PL181: Turning on");
#endif

    registers->clock = 0x1C6;
    registers->power = 0x86;

    // Send cmd 0 to set all cards into idle state
    _pl181_send_cmd(CMD_GO_IDLE_STATE | MMC_CMD_ENABLE_MASK, 0x0);

    // Voltage Check
    _pl181_send_cmd(8 | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, 0x1AA);
    if ((registers->response[0] & 0x1AA) != 0x1AA) {
#ifdef DEBUG_PL181
        log_error("PL181: Can't set voltage!");
#endif
        return -1;
    }

    if (_pl181_send_app_cmd(CMD_SD_SEND_OP_COND | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, 1 << 30 | 0x1AA)) {
#ifdef DEBUG_PL181
        log_error("PL181: Can't send APP_CMD!");
#endif
        return -1;
    }

    bool ishc = 0;

    if (registers->response[0] & 1 << 30) {
        ishc = 1;
#ifdef DEBUG_PL181
        log("PL181: ishc = 1");
#endif
    }

    _pl181_send_cmd(CMD_ALL_SEND_CID | MMC_CMD_ENABLE_MASK | MMC_CMD_LONG_RESP_MASK, 0x0);
    _pl181_send_cmd(CMD_SET_RELATIVE_ADDR | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, 0x0);

    // Get the card RCA from the response it is the top 16 bits of the 32 bit response
    uint32_t rca = (registers->response[0] & 0xFFFF0000);

    _pl181_send_cmd(CMD_SEND_CSD | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK | MMC_CMD_LONG_RESP_MASK, rca);
    uint32_t resp1 = registers->response[1];
    uint32_t capacity = ((resp1 >> 8) & 0x3) << 10;
    capacity |= (resp1 & 0xFF) << 2;
    capacity = 256 * 1024 * (capacity + 1);

    _pl181_register_device(rca, ishc, capacity);

    drive_desc->read = _pl181_read_block;
    return 0;
}