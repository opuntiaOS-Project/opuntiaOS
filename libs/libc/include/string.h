#ifndef _LIBC_STRING_H
#define _LIBC_STRING_H

#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

void* memset(void* dest, uint8_t fll, uint32_t nbytes);
void* memmove(void* dest, const void* src, uint32_t nbytes);
void* memcpy(void* dest, const void* src, uint32_t nbytes);
void* memccpy(void* dest, const void* src, uint8_t stop, uint32_t nbytes);
int memcmp(const uint8_t* src1, const uint8_t* src2, uint32_t nbytes);

uint32_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);

__END_DECLS

#endif // _LIBC_STRING_H
