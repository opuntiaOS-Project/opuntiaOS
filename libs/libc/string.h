#ifndef __oneOS__LibC__STRING_H
#define __oneOS__LibC__STRING_H

#include <sys/types.h>

void* memset(void* dest, uint8_t fll, uint32_t nbytes);
void* memmove(void* dest, const void* src, uint32_t nbytes);
void* memcpy(void* dest, const void* src, uint32_t nbytes);
void* memccpy(void* dest, const void* src, uint8_t stop, uint32_t nbytes);
int memcmp(const uint8_t* src1, const uint8_t* src2, uint32_t nbytes);
uint32_t strlen(const char* s);

#endif // __oneOS__LibC__STRING_H
