#include <utils/mem.h>

void memset(uint8_t* dest, uint8_t fll, uint32_t nbytes) {
    for (int i = 0; i < nbytes; ++i) {
        *(dest + i) = *(src + i);
    }
}

void memcpy(uint8_t* dest, uint8_t* src, uint32_t nbytes) {
    for (int i = 0; i < nbytes; ++i) {
        *(dest + i) = *(src + i);
    }
}

void memccpy(uint8_t* dest, uint8_t* src, uint8_t stop, uint32_t nbytes) {
    for (int i = 0; i < nbytes; ++i) {
        if (*(src + i) == stop) {
            return;
        }
        *(dest + i) = *(src + i);
    }
}
