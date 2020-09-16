#ifndef __oneOS__UTILS__MEM_H
#define __oneOS__UTILS__MEM_H

#include <types.h>

void memmove(uint8_t* dest, const uint8_t* src, uint32_t nbytes);
void memset(uint8_t* dest, uint8_t fll, uint32_t nbytes);
void memcpy(uint8_t* dest, const uint8_t* src, uint32_t nbytes);
int memcmp(const uint8_t* src1, const uint8_t* src2, uint32_t nbytes);
void memccpy(uint8_t* dest, const uint8_t* src, uint8_t stop, uint32_t nbytes);

char* kmem_bring_to_kernel(const char* data, uint32_t size);
char** kmem_bring_to_kernel_ptrarr(const char** data, uint32_t size);

#endif // __oneOS__UTILS__MEM_H
