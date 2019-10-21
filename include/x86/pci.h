#ifndef __oneOS__HARDWARECOMMUNICATION__PCI_H
#define __oneOS__HARDWARECOMMUNICATION__PCI_H

#include <types.h>
#include <x86/port.h>
#include <drivers/driverManager.h>
#include <drivers/display.h>


typedef struct {

    u_int8 bus;
    u_int8 device;
    u_int8 function;

    u_int16 vendor_id;
    u_int16 device_id;
    
    u_int8 class_id;
    u_int8 subclass_id; 
    u_int8 interface_id;
    u_int8 revision_id;

    u_int32 interrupt;
    uint32_t port_base;
} pcidd_t; // pci device desriptor

typedef enum {
    MEMORY_MAPPED = 0,
    INPUT_OUTPUT = 1
} BarType;

typedef struct {
    char prefetchable;
    uint8_t* address;
    BarType type;
} bar_t;


u_int32 pci_read(u_int16 bus, u_int16 device, u_int16 function, u_int32 offset);
void pci_write(u_int8 bus, u_int8 device, u_int8 function, u_int8 offset, u_int32 data);
char has_device_functions(u_int8 bus, u_int8 device);
void find_pci_devices();
pcidd_t get_device_desriptor(u_int8 bus, u_int8 device, u_int8 function);
bar_t get_bar(uint8_t bus, uint8_t device, uint8_t function, uint8_t bar_id);
driver_t* get_driver(pcidd_t dev);

#endif