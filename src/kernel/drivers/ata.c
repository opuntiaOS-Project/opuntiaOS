#include <drivers/ata.h>

// Private Members

uint8_t _ata_drives_count = 0;
driver_desc_t _ata_driver_info();

driver_desc_t _ata_driver_info() {
    driver_desc_t ata_desc;
    ata_desc.type = DRIVER_STORAGE;
    ata_desc.functions[DRIVER_STORAGE_ADD_DEVICE] = ata_add_new_device;
    ata_desc.functions[DRIVER_STORAGE_READ] = ata_read;
    ata_desc.functions[DRIVER_STORAGE_WRITE] = ata_write;
    ata_desc.functions[DRIVER_STORAGE_FLUSH] = ata_flush;
    ata_desc.pci_serve_class = 0x01;
    ata_desc.pci_serve_subclass = 0x05;
    ata_desc.pci_serve_vendor_id = 0x00;
    ata_desc.pci_serve_device_id = 0x00;
    return ata_desc;
}

void ata_add_new_device(device_t t_new_device) {
    bool is_master = t_new_device.device_desc.port_base >> 31;
    uint16_t port = t_new_device.device_desc.port_base & 0xFFF;
    ata_init(&_ata_drives[_ata_drives_count], port, is_master);
    ata_indentify(&_ata_drives[_ata_drives_count]);
    _ata_drives_count++;
    printf("Device added\n");
}

void ata_install() {
    // registering driver and passing info to it
    driver_install(_ata_driver_info());

    return _ata_drives_count;
}

void ata_init(ata_t *ata, uint32_t port, bool is_master){
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

bool ata_indentify(ata_t *ata) {
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
        return false;
    }

    // waiting for processing
    // while BSY is on
    while((status & 0x80) == 0x80) {
        status = port_8bit_in(ata->command_port);
    }
    // check if drive isn't ready to transer DRQ
    if ((status & 0x08) != 0x08) {
        printf("Doesn't ready to transfer DRQ");
        return false;
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
    return true;
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
    port_8bit_out(dev->lba_lo_port, 3);
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

void ata_read(ata_t *dev, uint32_t sectorNum, uint8_t *read_data) {
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

uint8_t ata_get_drives_count() {
    return _ata_drives_count;
}
