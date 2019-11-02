#include <drivers/ata.h>

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
        printf("Cmd isn't accepted");
        return;
    }

    // waiting for processing
    // while BSY is on
    while((status & 0x80) == 0x80) {
        status = port_8bit_in(ata->command_port);
    }

    // check if drive isn't ready to transer DRQ
    if ((status & 0x08) != 0x08) {
        printf("Don't ready for transport");
        return;
    }

    // transfering 256 bytes of data
    for (int i = 0; i < 256; i++) {
        uint16_t data = port_16bit_in(ata->data_port);
        char *text = "  \0";
        text[0] = (data >> 8) & 0xFF;
        text[1] = data & 0xFF;
        if (i == 1) {
            printf("Logical cylindres");
            printd(data);
            printf("\n");
        }
        if (i == 3) {
            printf("Logical heads");
            printd(data);
            printf("\n");
        }
        if (i == 6) {
            printf("Logical sectors");
            printd(data);
            printf("\n");
        }
        if (i == 49) {
            if ((data >> 8) & 0x1 == 1) {
                printf("Dma supported\n");
            }
            if ((data >> 9) & 0x1 == 1) {
                printf("Lba supported\n");
            }
        }
        //printf(text);
    }
    printf("\n");
}

void ata_write(ata_t *dev, char *data, int size) {

    uint8_t dev_config = 0xA0;
    // lba support
    dev_config |= (1 << 6);
    if (!dev->is_master) {
        dev_config |= (1 << 4);
    }

    port_8bit_out(dev->device_port, dev_config);
    port_8bit_out(dev->sector_count_port, 1);
    port_8bit_out(dev->lba_lo_port, 1);
    port_8bit_out(dev->lba_mid_port, 0);
    port_8bit_out(dev->lba_hi_port, 0);
    port_8bit_out(dev->error_port, 0);
    port_8bit_out(dev->command_port, 0x31);

    // waiting for processing
    // while BSY is on and no Errors
    uint8_t status = port_8bit_in(dev->command_port);
    while((status >> 7) & 1 == 1 && (status >> 0) & 1 != 1) {
        status = port_8bit_in(dev->command_port);
        printd(status);
        printf("\n");
    }

    // check if drive isn't ready to transer DRQ
    if ((status >> 0) & 1 == 1) {
        printf("Error");
        return;
    }


    for (int i = 0; i < size; i+=2) {
        uint16_t db = (data[i] << 8) + data[i+1];
        port_16bit_out(dev->data_port, db);
        char *text = "  \0";
        text[0] = (db >> 8) & 0xFF;
        text[1] = db & 0xFF;
        printf(text);
    }

    for (int i = size / 2; i < 256; i++) {
        port_16bit_out(dev->data_port, 0);
    }

}

void ata_read(ata_t *dev, uint8_t *read_data) {
    uint8_t dev_config = 0xA0;
    // lba support
    dev_config |= (1 << 6);
    if (!dev->is_master) {
        dev_config |= (1 << 4);
    }

    port_8bit_out(dev->device_port, dev_config);
    port_8bit_out(dev->sector_count_port, 1);
    port_8bit_out(dev->lba_lo_port, 1);
    port_8bit_out(dev->lba_mid_port, 0);
    port_8bit_out(dev->lba_hi_port, 0);
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
        printf("Error");
        return;
    }

    if (((status >> 3) & 1) == 0) {
        printf("No DRQ");
        return;
    }

    for (int i = 0; i < 256; i++) {
        uint16_t data = port_16bit_in(dev->data_port);
        read_data[2*i + 0] = (data >> 8) & 0xFF;
        read_data[2*i + 1] = (data >> 0) & 0xFF;
    }
}

void ata_flush(ata_t *dev) {
    uint8_t dev_config = 0xA0;
    // lba support
    dev_config |= (1 << 6);
    if (!dev->is_master) {
        dev_config |= (1 << 4);
    }
    port_8bit_out(dev->device_port, dev_config);
    port_8bit_out(dev->command_port, 0xE7);

    uint8_t status = port_8bit_in(dev->command_port);
    if (status == 0x00) {
        return;
    }

    while(((status >> 7) & 1) == 1 && ((status >> 0) & 1) != 1) {
        status = port_8bit_in(dev->command_port);
    }

    if (status & 0x01) {
        return;
    }
}
