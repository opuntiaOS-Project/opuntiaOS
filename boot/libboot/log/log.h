/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef _BOOT_LIBBOOT_LOG_LOG_H
#define _BOOT_LIBBOOT_LOG_LOG_H

#include <libboot/types.h>

typedef int (*uart_put_char_t)(uint8_t ch);

void log_init(uart_put_char_t ur);

int vsnprintf(char* s, size_t n, const char* format, va_list arg);
int vsprintf(char* s, const char* format, va_list arg);
int snprintf(char* s, size_t n, const char* format, ...);
int sprintf(char* s, const char* format, ...);

int log(const char* format, ...);
int log_warn(const char* format, ...);
int log_error(const char* format, ...);
int log_not_formatted(const char* format, ...);

#endif // _BOOT_LIBBOOT_LOG_LOG_H