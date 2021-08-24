#include "_internal.h"
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_structs.h>
#include <sys/types.h>
#include <unistd.h>

static int _scanf_u32(_lookupch_callback lookupch, _getch_callback getch, void* callback_params, uint32_t* val)
{
    int read = 0;
    uint32_t res = 0;
    while (isdigit(lookupch(callback_params))) {
        res *= 10;
        char ch = getch(callback_params);
        res += (ch - '0');
        read++;
    }
    *val = res;
    return read;
}

static int _scanf_i32(_lookupch_callback lookupch, _getch_callback getch, void* callback_params, int* val)
{
    int read = 0;
    bool negative = 0;
    if (lookupch(callback_params) == '-') {
        negative = true;
        getch(callback_params);
        read++;
    }

    uint32_t tmp_u32;
    int read_imm = _scanf_u32(lookupch, getch, callback_params, &tmp_u32);
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

static int _scanf_internal(const char* format, _lookupch_callback lookupch, _getch_callback getch, void* callback_params, va_list arg)
{
    const char* p = format;
    int passed = 0;

    while (*p) {
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

            while (isspace(lookupch(callback_params))) {
                getch(callback_params);
                passed++;
            }

            switch (*p) {
            case 'i':
            case 'd':
                if (!l_arg) {
                    int* value = va_arg(arg, int*);
                    int read = _scanf_i32(lookupch, getch, callback_params, value);
                    passed += read;
                }
                break;
            case 'u':
                if (!l_arg) {
                    uint32_t* value = va_arg(arg, uint32_t*);
                    int read = _scanf_u32(lookupch, getch, callback_params, value);
                    passed += read;
                }
                break;
            }
            p++;
        } else {
            while (isspace(lookupch(callback_params))) {
                getch(callback_params);
                passed++;
            }
            if (lookupch(callback_params) != *p) {
                break;
            }
            p++;
            getch(callback_params);
            passed++;
        }
    }
    return passed;
}

struct buffer_params {
    const char* buf;
};
typedef struct buffer_params buffer_params_t;

static int lookup_callback_buffer(void* callback_params)
{
    buffer_params_t* bp = (buffer_params_t*)callback_params;
    if (!bp->buf) {
        return EOF;
    }
    return *bp->buf;
}

static int getch_callback_buffer(void* callback_params)
{
    buffer_params_t* bp = (buffer_params_t*)callback_params;
    if (!bp->buf) {
        return EOF;
    }
    char res = *bp->buf;
    bp->buf++;
    return res;
}

int vsscanf(const char* buf, const char* format, va_list arg)
{
    buffer_params_t bp = { .buf = buf };
    return _scanf_internal(format, lookup_callback_buffer, getch_callback_buffer, (void*)&bp, arg);
}

static int lookup_callback_stream(void* callback_params)
{
    FILE* file = (FILE*)callback_params;
    int res = fgetc(file);
    ungetc(res, file);
    return res;
}

static int getch_callback_stream(void* callback_params)
{
    FILE* file = (FILE*)callback_params;
    int res = fgetc(file);
    return res;
}

int vfscanf(FILE* stream, const char* format, va_list arg)
{
    return _scanf_internal(format, lookup_callback_stream, getch_callback_stream, (void*)stream, arg);
}

int sscanf(const char* buf, const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int count = vsscanf(buf, format, ap);
    va_end(ap);
    return count;
}

int scanf(const char* format, ...)
{
    va_list ap;
    va_start(ap, format);
    int count = vfscanf(stdin, format, ap);
    va_end(ap);
    return count;
}
