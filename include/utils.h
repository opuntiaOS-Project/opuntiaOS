#ifndef __oneOS__UTILS__UTILS_H
#define __oneOS__UTILS__UTILS_H

#include <types.h>

void memcpy(char* src, char* dest, unsigned int nbytes);
void htos(int hex, char str[]);
void dtos(int dec, char str[]);
void reverse(char s[]);
int strlen(char s[]);

#endif