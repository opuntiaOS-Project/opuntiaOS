/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libkern/libkern.h>

int stoi(void* strv, int len)
{
    char* str = (char*)strv;
    if (len > 9) {
        return 0;
    }
    int res = 0;
    int mult = 1;
    char* end = str + len - 1;
    while (end >= str) {
        res += (*end - '0') * mult;
        mult *= 10;
        end--;
    }
    return res;
}

void htos(uint32_t hex, char str[])
{
    int i = 0;
    if (hex == 0) {
        str[0] = '0';
        i = 1;
    }
    while (hex != 0) {
        int num = hex % 16;
        hex /= 16;
        if (num < 10) {
            str[i++] = num + '0';
        } else {
            str[i++] = num + 'a' - 10;
        }
    }
    str[i] = '\0';
    reverse(str);
}

void dtos(uint32_t dec, char str[])
{
    int i = 0;
    if (dec == 0) {
        str[0] = '0';
        i = 1;
    }
    while (dec != 0) {
        int num = dec % 10;
        dec /= 10;
        str[i++] = num + '0';
    }
    str[i] = '\0';
    reverse(str);
}

void reverse(char s[])
{
    int c, i, j;
    for (i = 0, j = strlen(s) - 1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

uint32_t strlen(const char* s)
{
    uint32_t i = 0;
    while (s[i] != '\0')
        ++i;
    return i;
}

int strcmp(const char* a, const char* b)
{
    while (*a == *b && *a != 0 && *b != 0) {
        a++;
        b++;
    }

    if (*a < *b) {
        return -1;
    }
    if (*a > *b) {
        return 1;
    }
    return 0;
}

int strncmp(const char* a, const char* b, uint32_t num)
{
    while (*a == *b && *a != 0 && *b != 0 && num) {
        a++;
        b++;
        num--;
    }

    if (!num) {
        return 0;
    }

    if (*a < *b) {
        return -1;
    }
    if (*a > *b) {
        return 1;
    }
    return 0;
}

bool str_validate_len(const char* s, uint32_t len)
{
    for (int i = 0; i < len + 1; i++) {
        if (s[i] == 0) {
            return true;
        }
    }
    return false;
}

/**
 * Ptr array utils
 */

uint32_t ptrarr_len(const char** s)
{
    uint32_t len = 0;
    while (s[len] != 0) {
        ++len;
    }
    return len;
}

bool ptrarr_validate_len(const char** s, uint32_t len)
{
    for (int i = 0; i < len + 1; i++) {
        if (s[i] == 0) {
            return true;
        }
    }
    return false;
}