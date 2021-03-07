#ifndef _KERNEL_LIBKERN_LIBKERN_H
#define _KERNEL_LIBKERN_LIBKERN_H

#include <libkern/kassert.h>
#include <libkern/mem.h>
#include <libkern/types.h>

int stoi(void* str, int len);
void htos(uint32_t hex, char str[]);
void dtos(uint32_t dec, char str[]);
void reverse(char s[]);
uint32_t strlen(const char* s);
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, uint32_t num);
bool str_validate_len(const char* c, uint32_t len);

uint32_t ptrarr_len(const char** s);
bool ptrarr_validate_len(const char** s, uint32_t len);

static inline int imax(int a, int b)
{
    return a > b ? a : b;
}
static inline int imin(int a, int b)
{
    return a < b ? a : b;
}
static inline uint32_t u32max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}
static inline uint32_t u32min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

#endif // _KERNEL_LIBKERN_LIBKERN_H
