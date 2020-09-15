#ifndef __oneOS__UTILS__UTILS_H
#define __oneOS__UTILS__UTILS_H

#include <types.h>

int stoi(char *str, int len);
void htos(uint32_t hex, char str[]);
void dtos(uint32_t dec, char str[]);
void reverse(char s[]);
uint32_t strlen(const char *s);
int strcmp(const char* a, const char* b);
int strncmp(const char* a, const char* b, uint32_t num);
bool str_validate_len(const char* c, uint32_t len);

uint32_t ptrarr_len(const char** s);
bool ptrarr_validate_len(const char** s, uint32_t len);

#endif
