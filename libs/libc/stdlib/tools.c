#include <stdlib.h>
#include <string.h>

int atoi(const char* s)
{
    // FIXME: Add overflow checks
    int res = 0;
    int len = strlen(s);
    int mul = 1;
    for (int i = 0; i < len; i++) {
        mul *= 10;
    }
    for (int i = 0; i < len; i++) {
        mul /= 10;
        res += (s[i] - '0') * mul;
    }
    return res;
}