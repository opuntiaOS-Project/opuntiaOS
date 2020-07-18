#include <utils.h>

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

    if (a < b) {
        return -1;
    } else if (a > b) {
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
    } else if (a < b) {
        return -1;
    } else if (a > b) {
        return 1;
    }

    return 0;
}
