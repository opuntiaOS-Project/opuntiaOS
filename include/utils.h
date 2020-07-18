#ifndef __oneOS__UTILS__UTILS_H
#define __oneOS__UTILS__UTILS_H

#include <types.h>

void htos(uint32_t hex, char str[]);
void dtos(uint32_t dec, char str[]);
void reverse(char s[]);
uint32_t strlen(const char *s);
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, uint32_t num);

#endif
