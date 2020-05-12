#include <x86/pci.h>

driver_desc_t _pci_driver_info();

driver_desc_t _pci_driver_info() {
    driver_desc_t pci_desc;
    pci_desc.type = DRIVER_BUS_CONTROLLER;
    pci_desc.auto_start = true;
    pci_desc.is_device_driver = false;
    pci_desc.is_device_needed = false;
    pci_desc.is_driver_needed = false;
    pci_desc.functions[DRIVER_BUS_CONTROLLER_FIND_DEVICE] = pci_find_devices;
    return pci_desc;
}

void pci_install() {
    driver_install(_pci_driver_info());
}

uint32_t pci_read(uint16_t bus, uint16_t device, uint16_t function, uint32_t offset) {
    uint32_t id =
        0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (offset & 0xFC);
    port_dword_out(0xCF8, id);
    uint32_t tmp = (uint32_t)(port_dword_in(0xCFC) >> (8 * (offset % 4)));
    return tmp;
}

void pci_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t data) {
    uint32_t bus32 = bus;
    uint32_t device32 = device;
    uint16_t function16 = function;
    uint32_t address = (1 << 31)
                    | (bus32 << 16)
                    | (device32 << 11)
                    | (function16 << 8)
                    | (offset & 0xFC);
    port_dword_out(0xCF8, address);
    port_dword_out(0xCFC, data);
}

char pci_has_device_functions(uint8_t bus, uint8_t device) {
    return pci_read(bus, device, 0, 0x0e) & (1<<7);
}

void pci_find_devices() {
    kprintf("Pci scanning\n");
    uint8_t bus, device, function;
    for (bus = 0; bus < 8; bus++){
        for (device = 0; device < 32; device++){
            uint8_t functions_count = pci_has_device_functions(bus, device)==0 ? 8 : 1;
            for (function = 0; function < functions_count; function++) {
                device_desc_t dev = pci_get_device_desriptor(bus, device, function);
                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xffff) {
                    continue;
                }

                for (uint8_t bar_id = 0; bar_id < 6; bar_id++) {
                    bar_t bar = pci_get_bar(bus, device, function, bar_id);
                    if (bar.address && (bar.type == INPUT_OUTPUT)) {
                        dev.port_base = (uint32_t)bar.address;
                    }
                }

                device_install(dev);

                kprintf("Vendor %x, devID %x, cl %x scl %x\n", dev.vendor_id, dev.device_id, dev.class_id, dev.subclass_id);
            }
        }
    }
}

device_desc_t pci_get_device_desriptor(uint8_t bus, uint8_t device, uint8_t function) {
    device_desc_t new_device;

    new_device.bus = bus;
    new_device.device = device;
    new_device.function = function;

    new_device.vendor_id = pci_read(bus, device, function, 0x00);
    new_device.device_id = pci_read(bus, device, function, 0x02);

    new_device.class_id = pci_read(bus, device, function, 0x0b);
    new_device.subclass_id = pci_read(bus, device, function, 0x0a);
    new_device.interface_id = pci_read(bus, device, function, 0x09);
    new_device.revision_id = pci_read(bus, device, function, 0x08);

    new_device.interrupt = pci_read(bus, device, function, 0x3c);

    return new_device;
}

bar_t pci_get_bar(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_id) {
    bar_t result;

    uint32_t header_type = pci_read(bus, device, function, 0x0e) & 0x7f;
    uint8_t max_bars = 6 - (header_type * 4);
    if (bar_id >= max_bars)
        return result;

    uint32_t bar_val = pci_read(bus, device, function, 0x10 + 4 * bar_id);
    result.type = (bar_val & 0x1) ? INPUT_OUTPUT : MEMORY_MAPPED;

    if (result.type == MEMORY_MAPPED) {

    } else {
        result.address = (uint32_t)((bar_val >> 2) << 2);
        result.prefetchable = 0;
    }
    return result;
}
