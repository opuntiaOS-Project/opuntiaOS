#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H

#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#define _IOFBF 0 /* setvbuf should set fully buffered */
#define _IOLBF 1 /* setvbuf should set line buffered */
#define _IONBF 2 /* setvbuf should set unbuffered */
#define BUFSIZ 1024 /* size of buffer used by setbuf */

struct __fbuf {
    char* base;
    char* ptr; /* current pointer */
    size_t size;
};
typedef struct __fbuf __fbuf_t;

struct __file {
    int _flags; /* flags, below; this FILE is free if 0 */
    int _file; /* fileno, if Unix descriptor, else -1 */
    size_t _r; /* read space left */
    size_t _w; /* write space left */
    __fbuf_t _rbuf; /* read buffer */
    __fbuf_t _wbuf; /* write buffer */
};
typedef struct __file FILE;

FILE* fopen(const char* filename, const char* mode);
int fclose(FILE* stream);

size_t fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
size_t fwrite(const void* ptr, size_t size, size_t nmemb, FILE* stream);
int fseek(FILE* stream, uint32_t offset, int whence);

__END_DECLS

#endif // _LIBC_STDIO_H