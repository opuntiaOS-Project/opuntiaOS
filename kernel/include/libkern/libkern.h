#ifndef _KERNEL_LIBKERN_LIBKERN_H
#define _KERNEL_LIBKERN_LIBKERN_H

#include <libkern/kassert.h>
#include <libkern/mem.h>
#include <libkern/types.h>

#define KB (1024)
#define MB (1024 * 1024)

/**
 * SHORTCUTS
 */

#define TEST_FLAG(val, flag) ((val & flag) == flag)

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

#ifndef max
#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })
#endif /* max */

#ifndef min
#define min(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })
#endif /* min */

#endif // _KERNEL_LIBKERN_LIBKERN_H
