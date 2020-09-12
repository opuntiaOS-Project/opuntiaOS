#ifndef __oneOS__UTILS__LOG_H
#define __oneOS__UTILS__LOG_H

#include <types.h>

void logger_setup();
void log(const char* format, ...);
void log_warn(const char* format, ...);
void log_error(const char* format, ...);

#endif //__oneOS__UTILS__LOG_H