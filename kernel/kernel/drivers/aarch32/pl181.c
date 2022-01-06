/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <drivers/aarch32/pl181.h>
#include <libkern/log.h>
#include <mem/kmemzone.h>
#include <mem/vmm.h>

// #define DEBUG_PL181

static sd_card_t sd_cards[MAX_DEVICES_COUNT];
static kmemzone_t mapped_zone;
static volatile pl181_registers_t* registers;

static inline uintptr_t _pl181_mmio_paddr()
{
    return (uintptr_t)PL181_BASE;
}

static inline int _pl181_map_itself()
{
    uintptr_t mmio_paddr = _pl181_mmio_paddr();

    mapped_zone = kmemzone_new(sizeof(pl181_registers_t));
    vmm_map_page(mapped_zone.start, mmio_paddr, PAGE_DEVICE);
    registers = (pl181_registers_t*)mapped_zone.ptr;
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

static int _pl181_read_block(device_t* device, uint32_t lba_like, void* read_data)
{
    sd_card_t* sd_card = &sd_cards[device->id];
    uint32_t* read_data32 = (uint32_t*)read_data;
    uint32_t bytes_read = 0;

    registers->data_length = PL181_SECTOR_SIZE; // Set length of bytes to transfer
    registers->data_control = 0b11; // Enable dpsm and set direction from card to host

    if (sd_card->ishc) {
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

static int _pl181_write_block(device_t* device, uint32_t lba_like, void* write_data)
{
    sd_card_t* sd_card = &sd_cards[device->id];
    uint32_t* write_data32 = (uint32_t*)write_data;
    uint32_t bytes_written = 0;

    registers->data_length = PL181_SECTOR_SIZE; // Set length of bytes to transfer
    registers->data_control = 0b01; // Enable dpsm and set direction from host to card

    if (sd_card->ishc) {
        _pl181_send_cmd(CMD_WRITE_SINGLE_BLOCK | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, lba_like);
    } else {
        _pl181_send_cmd(CMD_WRITE_SINGLE_BLOCK | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, lba_like * PL181_SECTOR_SIZE);
    }

    while (registers->status & MMC_STAT_TRANSMIT_FIFO_EMPTY_MASK) {
        registers->fifo_data[0] = *write_data32;
        write_data32++;
        bytes_written += 4;
    }
    return bytes_written;
}

static void _pl181_add_new_device(device_t* new_device)
{
    bool ishc = new_device->device_desc.args[0] & 1;
    uint32_t rca = new_device->device_desc.args[0] & 0xFFFF0000;
    sd_cards[new_device->id].rca = rca;
    sd_cards[new_device->id].ishc = ishc;
    sd_cards[new_device->id].capacity = new_device->device_desc.args[1];
    _pl181_select_card(rca);
    _pl181_send_cmd(CMD_SET_SECTOR_SIZE | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, PL181_SECTOR_SIZE);
    if (registers->response[0] != 0x900) {
        log_error("PL181(pl181_add_new_device): Can't set sector size");
    }
}

static void _pl181_add_device(uint32_t rca, bool ishc, uint32_t capacity)
{
    device_desc_t new_device = { 0 };
    new_device.class_id = 0x08;
    new_device.subclass_id = 0x05;
    new_device.interface_id = 0;
    new_device.revision_id = 0;
    new_device.port_base = 0;
    new_device.interrupt = 0;
    new_device.args[0] = (rca & 0xFFFF0000) | ishc;
    new_device.args[1] = capacity;
    device_install(new_device);
}

static uint32_t _pl181_get_capacity(device_t* device)
{
    sd_card_t* sd_card = &sd_cards[device->id];
    return sd_card->capacity;
}

static driver_desc_t _pl181_driver_info()
{
    driver_desc_t ata_desc = { 0 };
    ata_desc.type = DRIVER_STORAGE_DEVICE;
    ata_desc.auto_start = false;
    ata_desc.is_device_driver = true;
    ata_desc.is_device_needed = false;
    ata_desc.is_driver_needed = false;
    ata_desc.functions[DRIVER_NOTIFICATION] = 0;
    ata_desc.functions[DRIVER_STORAGE_ADD_DEVICE] = _pl181_add_new_device;
    ata_desc.functions[DRIVER_STORAGE_READ] = _pl181_read_block;
    ata_desc.functions[DRIVER_STORAGE_WRITE] = _pl181_write_block;
    ata_desc.functions[DRIVER_STORAGE_FLUSH] = 0;
    ata_desc.functions[DRIVER_STORAGE_CAPACITY] = _pl181_get_capacity;
    ata_desc.pci_serve_class = 0x08;
    ata_desc.pci_serve_subclass = 0x05;
    ata_desc.pci_serve_vendor_id = 0x00;
    ata_desc.pci_serve_device_id = 0x00;
    return ata_desc;
}

void pl181_install()
{
    if (_pl181_map_itself()) {
#ifdef DEBUG_PL181
        log_error("PL181: Can't map itself!");
#endif
        return;
    }

    driver_install(_pl181_driver_info(), "pl181");
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
        return;
    }

    if (_pl181_send_app_cmd(CMD_SD_SEND_OP_COND | MMC_CMD_ENABLE_MASK | MMC_CMD_RESP_MASK, 1 << 30 | 0x1AA)) {
#ifdef DEBUG_PL181
        log_error("PL181: Can't send APP_CMD!");
#endif
        return;
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

    _pl181_add_device(rca, ishc, capacity);
}