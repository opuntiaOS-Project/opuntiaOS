#include <types.h>

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
} pcidd_t; // pci device desriptor


u_int32 pci_read(u_int16 bus, u_int16 device, u_int16 function, u_int32 offset);
void pci_write(u_int8 bus, u_int8 device, u_int8 function, u_int8 offset, u_int32 data);
char has_device_functions(u_int8 bus, u_int8 device);
void find_devices();
pcidd_t get_device_desriptor(u_int8 bus, u_int8 device, u_int8 function);

