#include <string.h>

void* memset(void* dest, uint8_t fll, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *((uint8_t*)dest + i) = fll;
    }
    return dest;
}

void* memmove(void* dest, const void* src, uint32_t nbytes)
{
    if (src > dest) {
        for (int i = 0; i < nbytes; ++i) {
            *((uint8_t*)dest + i) = *((uint8_t*)src + i);
        }
    } else {
        for (int i = nbytes - 1; i >= 0; --i) {
            *((uint8_t*)dest + i) = *((uint8_t*)src + i);
        }
    }
    return dest;
}

void* memcpy(void* dest, const void* src, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *((uint8_t*)dest + i) = *((uint8_t*)src + i);
    }
    return dest;
}

void* memccpy(void* dest, const void* src, uint8_t stop, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *((uint8_t*)dest + i) = *((uint8_t*)src + i);
        if (*((uint8_t*)src + i) == stop) {
            return ((uint8_t*)dest + i + 1);
        }
    }
    return NULL;
}

int memcmp(const uint8_t* src1, const uint8_t* src2, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        if (*(src1 + i) < *(src2 + i)) {
            return -1;
        }
        if (*(src1 + i) > *(src2 + i)) {
            return 1;
        }
    }
    return 0;
}

uint32_t strlen(const char* s)
{
    uint32_t i = 0;
    while (s[i] != '\0')
        ++i;
    return i;
}