#ifndef __oneOS__DRIVERS__ATA_H
#define __oneOS__DRIVERS__ATA_H

#include <types.h>
#include <x86/port.h>
#include <drivers/display.h>
#include <mem/malloc.h>
#include <drivers/driver_manager.h>

#define ATA_MAX_DRIVES_COUNT 2

typedef struct {                 // LBA28 | LBA48
    uint32_t data_port;          // 16bit | 16 bits
    uint32_t error_port;         // 8 bit | 16 bits
    uint32_t sector_count_port;  // 8 bit | 16 bits
    uint32_t lba_lo_port;        // 8 bit | 16 bits
    uint32_t lba_mid_port;       // 8 bit | 16 bits
    uint32_t lba_hi_port;        // 8 bit | 16 bits
    uint32_t device_port;        // 8 bit
    uint32_t command_port;       // 8 bit
    uint32_t control_port;       // 8 bit
    bool is_master;
} ata_t;

ata_t _ata_drives[ATA_MAX_DRIVES_COUNT];

void ata_add_new_device(device_t t_new_device);

void ata_install();
void ata_init(ata_t *ata, uint32_t port, bool is_master);
bool ata_indentify(ata_t *ata);
void ata_write(ata_t *dev, char *data, int size);
void ata_read(ata_t *dev, uint32_t sectorNum, uint8_t *read_data);
void ata_flush(ata_t *dev);

#endif //__oneOS__DRIVERS__ATA_H
