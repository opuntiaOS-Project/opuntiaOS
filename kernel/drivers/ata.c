#include <ata.h>

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
    port_8bit_out(ata->control_port, 0);
    
    port_8bit_out(ata->device_port, 0xA0);
    uint8_t status = port_8bit_in(ata->command_port);
    if (status == 0xFF) {
        printf("Err1");
        return;
    }
    
    port_8bit_out(ata->device_port, ata->is_master ? 0xA0 : 0xB0);
    port_8bit_out(ata->sector_count_port, 0);
    port_8bit_out(ata->lba_lo_port, 0);
    port_8bit_out(ata->lba_mid_port, 0);
    port_8bit_out(ata->lba_hi_port, 0);
    port_8bit_out(ata->command_port, 0xEC);
    
    status = port_8bit_in(ata->command_port);
    if (status == 0x00) {
        printf("Err2");
        return;
    }
    
    while(((status & 0x80) == 0x80)
       && ((status & 0x01) != 0x01))
        status = port_8bit_in(ata->command_port);
        
    if (status & 0x01) {
        printf("err3");
        return;
    }
    
    for (int i = 0; i < 256; i++) {
        uint16_t data = port_16bit_in(ata->data_port);
        char *text = "  \0";
        printd(data);
        text[0] = (data >> 8) & 0xFF;
        text[1] = data & 0xFF;
        printf(text);
    }
    printf("\n");
}