#ifndef _BOOT_X86_STAGE2_DRIVERS_ATA_H
#define _BOOT_X86_STAGE2_DRIVERS_ATA_H

#include "port.h"
#include <libboot/abi/drivers.h>
#include <libboot/types.h>

typedef struct { // LBA28 | LBA48
    uint32_t data_port; // 16bit | 16 bits
    uint32_t error_port; // 8 bit | 16 bits
    uint32_t sector_count_port; // 8 bit | 16 bits
    uint32_t lba_lo_port; // 8 bit | 16 bits
    uint32_t lba_mid_port; // 8 bit | 16 bits
    uint32_t lba_hi_port; // 8 bit | 16 bits
    uint32_t device_port; // 8 bit
    uint32_t command_port; // 8 bit
    uint32_t control_port; // 8 bit
    char is_master;
} ata_t;

void init_ata(uint32_t port, char is_master);
int indentify_ata_device(drive_desc_t* drive_desc);
int ata_read(uint32_t sector, uint8_t* read_to);

#endif