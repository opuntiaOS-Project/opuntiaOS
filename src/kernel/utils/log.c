#include <drivers/x86/uart.h>
#include <log.h>

static void _log_string(const char* str)
{
    while (*str) {
        uart_write(COM1, *str);
        str++;
    }
}

static void _log_dec(uint32_t dec)
{
    uint32_t pk = 1000000000;
    bool was_not_zero = 0;
    while (pk > 0) {
        uint32_t pp = dec / pk;
        if (was_not_zero || pp > 0) {
            uart_write(COM1, pp + '0');
            was_not_zero = 1;
        }
        dec -= pp * pk;
        pk /= 10;
    }
    if (!was_not_zero) {
        uart_write(COM1, '0');
    }
}

static void _log_hex(uint32_t hex)
{
    uint32_t pk = (uint32_t)0x10000000;
    bool was_not_zero = 0;

    if (hex == 0) {
        uart_write(COM1, '0');
    } else {
        uart_write(COM1, '0');
        uart_write(COM1, 'x');
    }

    while (pk > 0) {
        uint32_t pp = hex / pk;
        if (was_not_zero || pp > 0) {
            if (pp >= 10) {
                uart_write(COM1, pp - 10 + 'A');
            } else {
                uart_write(COM1, pp + '0');
            }
            was_not_zero = 1;
        }
        hex -= pp * pk;
        pk /= 16;
    }
}

void logger_setup()
{
    uart_setup(COM1);
}

void log(const char* format, ...)
{
    _log_string("\033[1;37m[LOG]\033[0m  ");
    char* ptr_to_next_arg = (char*)((uint32_t)&format + sizeof(void*));
    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == 'x') {
                uint32_t* arg = (uint32_t*)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(uint32_t);
                _log_hex(*arg);
            } else if (*format == 'd') {
                uint32_t* arg = (uint32_t*)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(uint32_t);
                _log_dec(*arg);
            } else if (*format == 's') {
                char** arg = (char**)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(char**);
                _log_string(*arg);
            }
        } else {
            uart_write(COM1, *format);
        }
        format++;
    }
    if (*(format - 1) != '\n') {
        uart_write(COM1, '\n');
    }
}

void log_warn(const char* format, ...)
{
    _log_string("\033[1;33m[WARN]\033[0m ");
    char* ptr_to_next_arg = (char*)((uint32_t)&format + sizeof(void*));
    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == 'x') {
                uint32_t* arg = (uint32_t*)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(uint32_t);
                _log_hex(*arg);
            } else if (*format == 'd') {
                uint32_t* arg = (uint32_t*)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(uint32_t);
                _log_dec(*arg);
            } else if (*format == 's') {
                char** arg = (char**)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(char**);
                _log_string(*arg);
            }
        } else {
            uart_write(COM1, *format);
        }
        format++;
    }
    if (*(format - 1) != '\n') {
        uart_write(COM1, '\n');
    }
}

void log_error(const char* format, ...)
{
    _log_string("\033[1;31m[ERR]\033[0m  ");
    char* ptr_to_next_arg = (char*)((uint32_t)&format + sizeof(void*));
    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == 'x') {
                uint32_t* arg = (uint32_t*)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(uint32_t);
                _log_hex(*arg);
            } else if (*format == 'd') {
                uint32_t* arg = (uint32_t*)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(uint32_t);
                _log_dec(*arg);
            } else if (*format == 's') {
                char** arg = (char**)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(char**);
                _log_string(*arg);
            }
        } else {
            uart_write(COM1, *format);
        }
        format++;
    }
    if (*(format - 1) != '\n') {
        uart_write(COM1, '\n');
    }
}

void log_char(char c)
{
    uart_write(COM1, c);
}

void log_not_formatted(const char* format, ...)
{
    char* ptr_to_next_arg = (char*)((uint32_t)&format + sizeof(void*));
    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == 'x') {
                uint32_t* arg = (uint32_t*)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(uint32_t);
                _log_hex(*arg);
            } else if (*format == 'd') {
                uint32_t* arg = (uint32_t*)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(uint32_t);
                _log_dec(*arg);
            } else if (*format == 's') {
                char** arg = (char**)ptr_to_next_arg;
                ptr_to_next_arg += sizeof(char**);
                _log_string(*arg);
            }
        } else {
            uart_write(COM1, *format);
        }
        format++;
    }
}