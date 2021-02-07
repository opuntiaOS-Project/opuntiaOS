#ifndef __oneOS__DRIVERS__RTC_H
#define __oneOS__DRIVERS__RTC_H

#include <types.h>

void rtc_load_time(uint8_t* secs, uint8_t* mins, uint8_t* hrs, uint8_t* day, uint8_t* month, uint32_t* year);

#endif /* __oneOS__DRIVERS__RTC_H */