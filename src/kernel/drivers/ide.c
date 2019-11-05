#include <drivers/ide.h>

driver_desc_t _ide_driver_info();
uint8_t _ide_drives_count = 0;
ata_t _ide_drives[2];

driver_desc_t _ide_driver_info() {
    driver_desc_t ide_desc;
    ide_desc.type = DRIVER_CONTROLLER;
    ide_desc.pci_serve_class = 0x01;
    ide_desc.pci_serve_subclass = 0x01;
    ide_desc.pci_serve_vendor_id = 0x00;
    ide_desc.pci_serve_device_id = 0x00;
    return ide_desc;
}

void ide_install() {
    uint32_t ask_ports[] = {0x1F0, 0x1F0};
    bool is_masters[] = {true, false};
    for (uint8_t i = 0; i < 2; i++) {
        ata_init(&_ide_drives[_ide_drives_count], ask_ports[i], is_masters[i]);
        if (ata_indentify(&_ide_drives[_ide_drives_count])) {
            _ide_drives_count++;
            pcidd_t new_device;
            new_device.class_id = 0x01;
            new_device.subclass_id = 0x05;
            new_device.interface_id = 0;
            new_device.revision_id = 0;
            new_device.port_base = ask_ports[i] | (1 << 31);
            new_device.interrupt = IRQ14;
            device_install(new_device);
        }
    }
}
