#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
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

long strtol(const char* nptr, char** endptr, int base)
{
    const char* p = nptr;
    const char* endp = nptr;
    bool negative = false;
    bool overflow = false;
    unsigned long n = 0;

    if (base < 0 || base == 1 || base > 36) {
        set_errno(EINVAL);
        return 0;
    }

    while (isspace(*p)) {
        p++;
    }

    if (*p == '+') {
        p++;
    } else if (*p == '-') {
        negative = 1;
        p++;
    }

    if (*p == '0') {
        p++;
        endp = p;
        if (base == 16 && (*p == 'X' || *p == 'x')) {
            p++;
        } else if (base == 0) {
            if (*p == 'X' || *p == 'x') {
                base = 16, p++;
            } else {
                base = 8;
            }
        }
    } else if (base == 0) {
        base = 10;
    }

    unsigned long cutoff = (negative) ? -(LONG_MIN / base) : LONG_MAX / base;
    int cutlim = (negative) ? -(LONG_MIN % base) : LONG_MAX % base;

    for (;;) {
        int digit;
        if (*p >= 'a') {
            digit = (*p - 'a') + 10;
        } else if (*p >= 'A') {
            digit = *p - 'A' + 10;
        } else if (*p <= '9') {
            digit = *p - '0';
        } else {
            break;
        }
        if (digit < 0 || digit >= base) {
            break;
        }
        p++;
        endp = p;

        if (overflow) {
            if (endptr) {
                continue;
            }
            break;
        }

        if (n > cutoff || (n == cutoff && digit > cutlim)) {
            overflow = 1;
            continue;
        }
        n = n * base + digit;
    }

    if (endptr) {
        *endptr = (char*)endp;
    }

    if (overflow) {
        set_errno(ERANGE);
        return ((negative) ? LONG_MIN : LONG_MAX);
    }
    return (long)((negative) ? -n : n);
}

#define SWAPARR(a, b, size)          \
    do {                             \
        size_t __size = (size);      \
        char *__a = (a), *__b = (b); \
        do {                         \
            char __tmp = *__a;       \
            *__a++ = *__b;           \
            *__b++ = __tmp;          \
        } while (--__size > 0);      \
    } while (0)

void __qsortimpl(void* vbase, size_t n, size_t size, int (*compar)(const void*, const void*), int l, int r)
{
    char* base = (char*)vbase;

    if (l >= r) {
        return;
    }

    int lm = l - 1;
    for (int rm = l; rm < r; rm++) {
        if (compar(&base[rm * size], &base[r * size]) < 0) {
            lm++;
            SWAPARR(&base[lm * size], &base[rm * size], size);
        }
    }
    SWAPARR(&base[(lm + 1) * size], &base[r * size], size);

    __qsortimpl(vbase, n, size, compar, l, lm);
    __qsortimpl(vbase, n, size, compar, lm + 2, r);
}

void qsort(void* vbase, size_t n, size_t size, int (*compar)(const void*, const void*))
{
    __qsortimpl(vbase, n, size, compar, 0, n - 1);
}
