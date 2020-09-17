#ifndef __oneOS__LibC__STRING_H
#define __oneOS__LibC__STRING_H

#include <sys/types.h>

extern int errno;

void memset(uint8_t* dest, uint8_t fll, uint32_t nbytes);
void memmove(uint8_t* dest, const uint8_t* src, uint32_t nbytes);
void memcpy(uint8_t* dest, const uint8_t* src, uint32_t nbytes);
int memcmp(const uint8_t* src1, const uint8_t* src2, uint32_t nbytes);
void memccpy(uint8_t* dest, const uint8_t* src, uint8_t stop, uint32_t nbytes);

uint32_t strlen(const char*);

#endif // __oneOS__LibC__STRING_H
