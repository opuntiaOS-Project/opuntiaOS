#ifndef __oneOS__HARDWARECOMMUNICATION__PCI_H
#define __oneOS__HARDWARECOMMUNICATION__PCI_H

#include <types.h>
#include <x86/port.h>
#include <drivers/driver_manager.h>
#include <drivers/display.h>

typedef struct {
    uint8_t bus;
    uint8_t device;
    uint8_t function;

    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t class_id;
    uint8_t subclass_id;
    uint8_t interface_id;
    uint8_t revision_id;

    uint32_t interrupt;
    uint32_t port_base;
} pcidd_t; // pci device desriptor

typedef enum {
    MEMORY_MAPPED = 0,
    INPUT_OUTPUT = 1
} BarType;

typedef struct {
    char prefetchable;
    uint32_t address;
    BarType type;
} bar_t;

void pci_install();
uint32_t pci_read(uint16_t bus, uint16_t device, uint16_t function, uint32_t offset);
void pci_write(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t data);
char pci_has_device_functions(uint8_t bus, uint8_t device);
void pci_find_devices();
device_desc_t pci_get_device_desriptor(uint8_t bus, uint8_t device, uint8_t function);
bar_t pci_get_bar(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_id);

#endif
