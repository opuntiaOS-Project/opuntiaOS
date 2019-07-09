#include <pci.h>
#include <port.h>
#include <display.h>

u_int32 pci_read(u_int16 bus, u_int16 device, u_int16 function, u_int32 offset) {
    u_int32 id =
        0x1 << 31
        | ((bus & 0xFF) << 16)
        | ((device & 0x1F) << 11)
        | ((function & 0x07) << 8)
        | (offset & 0xFC);
    port_dword_out(0xCF8, id);
    u_int32 tmp = (u_int32)(port_dword_in(0xCFC) >> (8 * (offset % 4)));
    return tmp;
}

void pci_write(u_int8 bus, u_int8 device, u_int8 function, u_int8 offset, u_int32 data) {
    u_int32 bus32 = bus;
    u_int32 device32 = device;
    u_int16 function16 = function;
    u_int32 address = (1 << 31)
                    | (bus32 << 16)
                    | (device32 << 11)
                    | (function16 << 8)
                    | (offset & 0xFC);
    port_dword_out(0xCF8, address);
    port_dword_out(0xCFC, data);
}


char has_device_functions(u_int8 bus, u_int8 device) {
    return pci_read(bus, device, 0, 0x0e) & (1<<7);
}

void find_devices() {
    printf("Pci scaing\n\n");
    u_int8 bus, device, function;
    for (bus = 0; bus < 8; bus++){
        for (device = 0; device < 32; device++){
            u_int8 functions_count = has_device_functions(bus, device)==0 ? 8 : 1;
            for (function = 0; function < functions_count; function++) {
                pcidd_t dev = get_device_desriptor(bus, device, function);
                if (dev.vendor_id == 0x0000 || dev.vendor_id == 0xffff) {
                    break;
                }
                //if (dev.class_id == 0x00) continue;
                printf("PCI BUS ");
                printh(bus & 0xFF);
                
                printf(", DEVICE ");
                printh(device & 0xFF);

                printf(", FUNCTION ");
                printh(function & 0xFF);
                
                printf(" = VENDOR ");
                printh((dev.vendor_id & 0xFF00) >> 8);
                printh(dev.vendor_id & 0xFF);
                printf(", DEVICE ");
                printh((dev.device_id & 0xFF00) >> 8);
                printh(dev.device_id & 0xFF);
                printf("\n");
            }
        }
    }
}

pcidd_t get_device_desriptor(u_int8 bus, u_int8 device, u_int8 function) {
    pcidd_t new_device;

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

