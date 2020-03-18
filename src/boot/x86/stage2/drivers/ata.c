#include "ata.h"

void init_ata(ata_t *ata, uint32_t port, char is_master){
    ata->is_master = is_master;
    ata->data_port = port;
    ata->error_port = port + 0x1;
    ata->sector_count_port = port + 0x2;
    ata->lba_lo_port = port + 0x3;
    ata->lba_mid_port = port + 0x4;
    ata->lba_hi_port = port + 0x5;
    ata->device_port = port + 0x6;
    ata->command_port = port + 0x7;
    ata->control_port = port + 0x206;
} 

void indentify_ata_device(ata_t *ata) {
    port_8bit_out(ata->device_port, ata->is_master ? 0xA0 : 0xB0);
    port_8bit_out(ata->sector_count_port, 0);
    port_8bit_out(ata->lba_lo_port, 0);
    port_8bit_out(ata->lba_mid_port, 0);
    port_8bit_out(ata->lba_hi_port, 0);
    port_8bit_out(ata->command_port, 0xEC);
    
    // check the acceptance of a command
    uint8_t status = port_8bit_in(ata->command_port);
    if (status == 0x00) {
        // printf("Cmd isn't accepted");
        return;
    }
    
    // waiting for processing
    // while BSY is on
    while((status & 0x80) == 0x80) {
        status = port_8bit_in(ata->command_port);
    }
        
    // check if drive isn't ready to transer DRQ
    if ((status & 0x08) != 0x08) {
        // printf("Don't ready for transport");
        return;
    }
    
    // transfering 256 bytes of data
    for (int i = 0; i < 256; i++) {
        uint16_t data = port_16bit_in(ata->data_port);
        char *text = "  \0";
        text[0] = (data >> 8) & 0xFF;
        text[1] = data & 0xFF;
        if (i == 1) {
            // printf("Logical cylindres");
            // printd(data);
            // printf("\n");
        }
        if (i == 3) {
            // printf("Logical heads");
            // printd(data);
            // printf("\n");
        }
        if (i == 6) {
            // printf("Logical sectors");
            // printd(data);
            // printf("\n");
        }
        if (i == 49) {
            if ((data >> 8) & 0x1 == 1) {
                // printf("Dma supported\n");
            }
            if ((data >> 9) & 0x1 == 1) {
                // printf("Lba supported\n");
            }
        }
    }
}

// Returning 2 head read bytes (used to get size of kernel) 
uint16_t ata_read_to_ram(ata_t *dev, uint32_t sectorNum, uint32_t toRam, uint8_t offset) {
    uint8_t dev_config = 0xA0;
    // lba support
    dev_config |= (1 << 6);
    if (!dev->is_master) { 
        dev_config |= (1 << 4);
    }
    
    port_8bit_out(dev->device_port, dev_config);
    port_8bit_out(dev->sector_count_port, 1);
    port_8bit_out(dev->lba_lo_port,   sectorNum & 0x000000FF);
    port_8bit_out(dev->lba_mid_port, (sectorNum & 0x0000FF00) >> 8);
    port_8bit_out(dev->lba_hi_port,  (sectorNum & 0x00FF0000) >> 16);
    port_8bit_out(dev->error_port, 0);
    port_8bit_out(dev->command_port, 0x21);

    // waiting for processing
    // while BSY is on and no Errors
    uint8_t status = port_8bit_in(dev->command_port);
    while(((status >> 7) & 1) == 1 && ((status >> 0) & 1) != 1) {
        status = port_8bit_in(dev->command_port);
    }

    // check if drive isn't ready to transer DRQ
    if (((status >> 0) & 1) == 1) {
        return 0;
    }

    if (((status >> 3) & 1) == 0) {
        return 0;
    }

    uint8_t *ram = (uint8_t *)toRam;
    uint16_t result = 0;

    for (int i = 0; i < 256; i++) {
        uint16_t data = port_16bit_in(dev->data_port);
        if (i == 0) {
            result = data;
        }
        if (2 * i >= offset) {
            ram[2*i+1 - offset] = (data >> 8) & 0xFF;
            ram[2*i - offset] = data & 0xFF;
        }
    }
    return result;
}