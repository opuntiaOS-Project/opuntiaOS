#ifndef _KERNEL_LIBKERN_MEM_H
#define _KERNEL_LIBKERN_MEM_H

#include <libkern/types.h>

void* memset(void* dest, uint8_t fll, size_t nbytes);
void* memcpy(void* dest, const void* src, size_t nbytes);
void* memccpy(void* dest, const void* src, uint8_t stop, size_t nbytes);
void* memmove(void* dest, const void* src, size_t nbytes);
int memcmp(const void* src1, const void* src2, size_t nbytes);

char* kmem_bring_to_kernel(const char* data, size_t size);
char** kmem_bring_to_kernel_ptrarr(const char** data, size_t size);

#endif // _KERNEL_LIBKERN_MEM_H
