#include <utils.h>

void htos(uint32_t hex, char str[]) {
    int i = 0;
    if (hex == 0) {
        str[0] = '0';
        i = 1;
    }
    while(hex != 0) {
        int num = hex % 16;
        hex /= 16;
        if (num < 10) {
            str[i++] = num+'0';
        } else {
            str[i++] = num+'a'-10;
        }
    }
    str[i] = '\0';
    reverse(str);
}

void dtos(uint32_t dec, char str[]) {
    int i = 0;
    if (dec == 0) {
        str[0] = '0';
        i = 1;
    }
    while(dec != 0) {
        int num = dec % 10;
        dec /= 10;
        str[i++] = num+'0';
    }
    str[i] = '\0';
    reverse(str);
}

void reverse(char s[]) {
    int c, i, j;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int strlen(char s[]) {
    int i = 0;
    while (s[i] != '\0') ++i;
    return i;
}
