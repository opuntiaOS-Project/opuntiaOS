/**
 * Ata Stage2 driver. This is a lite version, since it can work with
 * only one drive at time. The @active_ata_drive is a boot drive.
 */

#include "ata.h"
#include "display.h"

ata_t active_ata_drive;

void init_ata(uint32_t port, char is_master) {
    active_ata_drive.is_master = is_master;
    active_ata_drive.data_port = port;
    active_ata_drive.error_port = port + 0x1;
    active_ata_drive.sector_count_port = port + 0x2;
    active_ata_drive.lba_lo_port = port + 0x3;
    active_ata_drive.lba_mid_port = port + 0x4;
    active_ata_drive.lba_hi_port = port + 0x5;
    active_ata_drive.device_port = port + 0x6;
    active_ata_drive.command_port = port + 0x7;
    active_ata_drive.control_port = port + 0x206;
} 

int indentify_ata_device(drive_desc_t *drive_desc) {
    port_8bit_out(active_ata_drive.device_port, active_ata_drive.is_master ? 0xA0 : 0xB0);
    port_8bit_out(active_ata_drive.sector_count_port, 0);
    port_8bit_out(active_ata_drive.lba_lo_port, 0);
    port_8bit_out(active_ata_drive.lba_mid_port, 0);
    port_8bit_out(active_ata_drive.lba_hi_port, 0);
    port_8bit_out(active_ata_drive.command_port, 0xEC);
    
    // check the acceptance of a command
    uint8_t status = port_8bit_in(active_ata_drive.command_port);
    if (status == 0x00) {
        // printf("Cmd isn't accepted");
        return -1;
    }
    
    // waiting for processing
    // while BSY is on
    while((status & 0x80) == 0x80) {
        status = port_8bit_in(active_ata_drive.command_port);
    }
        
    // check if drive isn't ready to transer DRQ
    if ((status & 0x08) != 0x08) {
        // printf("Don't ready for transport");
        return -1;
    }
    
    // transfering 256 bytes of data
    for (int i = 0; i < 256; i++) {
        uint16_t data = port_16bit_in(active_ata_drive.data_port);
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

    drive_desc->read = ata_read;
    printh((uint32_t)ata_read);
    printh((uint32_t)&ata_read);
    printh((uint32_t)drive_desc->read);

    return 0;
}

// Returning 2 head read bytes (used to get size of kernel) 
int ata_read(uint32_t sector, uint8_t* read_to) {
    uint8_t dev_config = 0xA0;
    // lba support
    dev_config |= (1 << 6);
    if (!active_ata_drive.is_master) { 
        dev_config |= (1 << 4);
    }
    
    port_8bit_out(active_ata_drive.device_port, dev_config);
    port_8bit_out(active_ata_drive.sector_count_port, 1);
    port_8bit_out(active_ata_drive.lba_lo_port,   sector & 0x000000FF);
    port_8bit_out(active_ata_drive.lba_mid_port, (sector & 0x0000FF00) >> 8);
    port_8bit_out(active_ata_drive.lba_hi_port,  (sector & 0x00FF0000) >> 16);
    port_8bit_out(active_ata_drive.error_port, 0);
    port_8bit_out(active_ata_drive.command_port, 0x21);

    // waiting for processing
    // while BSY is on and no Errors
    uint8_t status = port_8bit_in(active_ata_drive.command_port);
    while(((status >> 7) & 1) == 1 && ((status >> 0) & 1) != 1) {
        status = port_8bit_in(active_ata_drive.command_port);
    }

    // check if drive isn't ready to transer DRQ
    if (((status >> 0) & 1) == 1) {
        return -1;
    }

    if (((status >> 3) & 1) == 0) {
        return -1;
    }

    for (int i = 0; i < 256; i++) {
        uint16_t data = port_16bit_in(active_ata_drive.data_port);
        read_to[2*i+1] = (data >> 8) & 0xFF;
        read_to[2*i] = data & 0xFF;
    }

    return 0;
}