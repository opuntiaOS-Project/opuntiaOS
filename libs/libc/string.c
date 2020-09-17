#include <string.h>

void memset(uint8_t* dest, uint8_t fll, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *(dest + i) = fll;
    }
}

void memmove(uint8_t* dest, const uint8_t* src, uint32_t nbytes)
{
    if (src > dest) {
        for (int i = 0; i < nbytes; ++i) {
            *(dest + i) = *(src + i);
        }
    } else {
        for (int i = nbytes - 1; i >= 0; --i) {
            *(dest + i) = *(src + i);
        }
    }
}


void memcpy(uint8_t* dest, const uint8_t* src, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *(dest + i) = *(src + i);
    }
}

void memccpy(uint8_t* dest, const uint8_t* src, uint8_t stop, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        if (*(src + i) == stop) {
            return;
        }
        *(dest + i) = *(src + i);
    }
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