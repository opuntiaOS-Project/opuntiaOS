#ifndef _LIBOBJC_MEMORY_H
#define _LIBOBJC_MEMORY_H

#include <stdlib.h>

#define objc_malloc(...) malloc(__VA_ARGS__)
#define objc_realloc(...) realloc(__VA_ARGS__)
#define objc_calloc(...) calloc(__VA_ARGS__)
#define objc_free(...) free(__VA_ARGS__)

#endif // _LIBOBJC_MEMORY_H