#ifndef __oneOS__DRIVERS__DRIVERMANAGER_H
#define __oneOS__DRIVERS__DRIVERMANAGER_H

#include <types.h>

#define MAX_DRIVERS 256

typedef struct {
    uint8_t interrupt_line;
    uint32_t *activate;
    uint32_t *stop;
} driver_t; 

uint8_t drivers_cnt;
driver_t drivers[MAX_DRIVERS];

void register_drivers();
uint8_t register_driver(bool *activate_function(void), void *stop_function(void));
bool start_driver(uint8_t id);
void stop_driver(uint8_t id);
bool start_all_drivers();
void stop_all_drivers();

#endif