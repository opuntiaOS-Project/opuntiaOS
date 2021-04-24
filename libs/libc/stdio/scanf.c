#include "_internal.h"
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_structs.h>
#include <sys/types.h>
#include <unistd.h>

static int _scanf_u32(const char* buf, uint32_t* val)
{
    int read = 0;
    uint32_t res = 0;
    while (*buf && isdigit(*buf)) {
        res *= 10;
        res += (*buf - '0');
        buf++;
        read++;
    }
    *val = res;
    return read;
}

static int _scanf_i32(const char* buf, int* val)
{
    int read = 0;
    char negative = 0;
    if (*buf == '-') {
        negative = 1;
        buf++;
        read++;
    }

    uint32_t tmp_u32;
    int read_imm = _scanf_u32(buf, &tmp_u32);
    if (!read_imm) {
        return 0;
    }
    read += read_imm;

    int tmp_i32 = (int)tmp_u32;
    if (negative) {
        tmp_i32 = -tmp_i32;
    }
    *val = tmp_i32;

    return read;
}

int vsscanf(const char* buf, const char* format, va_list arg)
{
    const char* p = format;
    int passed = 0;

    while (*p && *format) {
        int l_arg = 0;
        int h_arg = 0;
        while (isspace(*p)) {
            p++;
        }
        if (*p == '%' && *(p + 1)) {
            // Reading arguments
        parse_args:
            p++;
            switch (*p) {
            case 'l':
                l_arg++;
                if (*(p + 1)) {
                    goto parse_args;
                }
                break;
            case 'h':
                h_arg++;
                if (*(p + 1)) {
                    goto parse_args;
                }
                break;
            default:
                break;
            }

            while (isspace(*buf)) {
                buf++;
                passed++;
            }

            switch (*p) {
            case 'i':
            case 'd':
                if (!l_arg) {
                    int* value = va_arg(arg, int*);
                    int read = _scanf_i32(buf, value);
                    buf += read;
                    passed += read;
                }
                break;
            case 'u':
                if (!l_arg) {
                    uint32_t* value = va_arg(arg, uint32_t*);
                    int read = _scanf_u32(buf, value);
                    buf += read;
                    passed += read;
                }
                break;
            }
            p++;
        } else {
            while (isspace(*buf)) {
                buf++;
                passed++;
            }
            if (*buf != *p) {
                break;
            }
            p++;
            buf++;
            passed++;
        }
    }
    return passed;
}

int sscanf(const char* str, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int count = vsscanf(str, fmt, ap);
    va_end(ap);
    return count;
}