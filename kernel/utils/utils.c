#include "utils.h"

void memcpy(char* src, char* dest, unsigned int nbytes) {
    int i;
    for (i = 0; i < nbytes; ++i) {
        *(dest + i) = *(src + i);
    }
}