/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *  + Contributed by bellrise <bellrise.dev@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
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
#define EOF (-1)
#define UNGOTC_EMPTY (-0x400)

/* Define stdin, stdout and stderr macros for better readability. */
#define STDIN 0
#define STDOUT 1
#define STDERR 2

typedef struct __fbuf __fbuf_t;
typedef struct __rwbuf __rwbuf_t;
typedef struct __file FILE;

extern FILE* stdin;
extern FILE* stdout;
extern FILE* stderr;

/* Open a file. Returns a struct representing an open file allocated on the
   heap. The possible mode combinations are: r, w, a, r+ and w+. Returns NULL
   if anything goes wrong. */
FILE* fopen(const char* filename, const char* mode);

/* Deallocate the file and close it. Any remaining data is flushed. Returns 0
   on success, or EOF on failure. */
int fclose(FILE* stream);

/* Read up to 'count' objects, each of 'size' size into 'buf' from 'stream' */
size_t fread(void* buf, size_t size, size_t count, FILE* stream);

/* Write up to 'count' objects, each of 'size' size into 'stream' from 'buf'. */
size_t fwrite(const void* buf, size_t size, size_t count, FILE* stream);

/* Set the file indicator to 'offset', starting from 'origin'. */
int fseek(FILE* stream, uint32_t offset, int origin);

/* Flush all buffers, writing all stored data into the stored device/file. */
int fflush(FILE* stream);

/* Put a single character to the given stream. 'putchar' will write the char
   to stdout. */
int fputc(int c, FILE* stream);
int putc(int c, FILE* stream) __use_instead("fputc");
int putchar(int c);

/* Put a string to the given stream with a newline at the end, 'puts' writes
   to stdout by default. */
int fputs(const char* s, FILE* stream);
int puts(const char* s);

/* Read/place back a char from the given stream. 'getchar' will get a single
   char from stdin. */
int fgetc(FILE* stream);
int getc(FILE* stream) __use_instead("fgets");
int getchar();
int ungetc(int c, FILE* stream);

/* Get a string from the given stream, of max 'size'. */
char* fgets(char* str, int size, FILE* stream);
char* gets(char* str);

/* Checks whether the end-of-File indicator associated with stream is set,
   returning a value different from zero if it is. */
int feof(FILE* stream);

/* Set the buffering mode of the stream. 'mode' can either be _IOFBF, _IOLBF
   or _IONBF. If 'buf' is NULL, resize the internal buffer to 'size'. If 'buf'
   is instead not NULL, it changes the internal buffer to the user provided
   one of size 'size'. Return -1 if something goes wrong. */
int setvbuf(FILE* stream, char* buf, int mode, size_t size);

/* Set the buffering mode to full buffering if 'buf' is provided. The minimum
   size of the buffer should be BUFSIZ, or it could lead to a buffer overflow.
   If 'buf' is NULL, buffering it turned off. */
void setbuf(FILE* stream, char* buf);

/* Set the buffering mode of the given stream to _IOLBF. */
void setlinebuf(FILE* stream);

/* Works the same as the C standard says, check there. */
int vsnprintf(char* buf, size_t n, const char* fmt, va_list arg);
int vsprintf(char* buf, const char* fmt, va_list arg);
int snprintf(char* buf, size_t n, const char* fmt, ...);
int sprintf(char* buf, const char* fmt, ...);
int fprintf(FILE* stream, const char* fmt, ...);
int printf(const char* fmt, ...);

/* Scan a buffer for the provided values. Works similarly to printf, but it
   reads instead of writes. 'scanf' reads from stdin instead of a buffer. Note
   that using these functions may be unsafe. Never trust the user! */
int vsscanf(const char* buf, const char* fmt, va_list arg);
int sscanf(const char* buf, const char* fmt, ...);
int scanf(const char* fmt, ...);

/* Dump information about the stream to stdout. */
int __stream_info(FILE* stream);

__END_DECLS

#endif // _LIBC_STDIO_H
