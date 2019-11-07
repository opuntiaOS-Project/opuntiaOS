#ifndef __oneOS__DRIVERS__ATA_H
#define __oneOS__DRIVERS__ATA_H

#include <types.h>
#include <x86/port.h>
#include <drivers/display.h>
#include <mem/malloc.h>
#include <drivers/driver_manager.h>

#define ATA_MAX_DRIVES_COUNT 2

typedef struct {            // LBA28 | LBA48
    uint32_t data;          // 16bit | 16 bits
    uint32_t error;         // 8 bit | 16 bits
    uint32_t sector_count;  // 8 bit | 16 bits
    uint32_t lba_lo;        // 8 bit | 16 bits
    uint32_t lba_mid;       // 8 bit | 16 bits
    uint32_t lba_hi;        // 8 bit | 16 bits
    uint32_t device;        // 8 bit
    uint32_t command;       // 8 bit
    uint32_t control;
} ata_ports_t;

typedef struct {
    ata_ports_t port;
    bool is_master;
    uint16_t cylindres;
    uint16_t heads;
    uint16_t sectors;
    bool dma;
    bool lba;
} ata_t;

ata_t _ata_drives[ATA_MAX_DRIVES_COUNT];

void ata_add_new_device(device_t *t_new_device);

void ata_install();
void ata_init(ata_t *ata, uint32_t port, bool is_master);
bool ata_indentify(ata_t *ata);
void ata_write(device_t *t_device, uint32_t sector, uint8_t *data, uint32_t size);
void ata_read(device_t *t_device, uint32_t sector, uint8_t *read_data);
void ata_flush(device_t *t_device);

#endif //__oneOS__DRIVERS__ATA_H
