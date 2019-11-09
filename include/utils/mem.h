#ifndef __oneOS__UTILS__MEM_H
#define __oneOS__UTILS__MEM_H

#include <types.h>

void memset(uint8_t* src, uint8_t fll, uint32_t nbytes);
void memcpy(uint8_t* dest, uint8_t* src, uint32_t nbytes);
void memccpy(uint8_t* dest, uint8_t* src, uint8_t stop, uint32_t nbytes);

#endif // __oneOS__UTILS__MEM_H
