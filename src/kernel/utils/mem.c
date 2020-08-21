#include <mem/kmalloc.h>
#include <utils/mem.h>

void memset(uint8_t* dest, uint8_t fll, uint32_t nbytes)
{
    for (int i = 0; i < nbytes; ++i) {
        *(dest + i) = fll;
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

char* kmem_bring_to_kernel(const char* data, uint32_t size)
{
    char* kdata = kmalloc(size);
    if (kdata) {
        memcpy(kdata, data, size);
    }
    return kdata;
}

char** kmem_bring_to_kernel_ptrarr(const char** data, uint32_t size)
{
    char** res = kmalloc(size * sizeof(char*));
    
    for (int i = 0; i < size; i++) {
        res[i] = kmem_bring_to_kernel(data[i], strlen(data[i]) + 1);
    }
    
    return res;
}
