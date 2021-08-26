/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *  + Contributed by bellrise <bellrise.dev@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include <bits/fcntl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define _IO_MAGIC 0xFBAD0000 /* Magic number */
#define _IO_MAGIC_MASK 0xFFFF0000
#define _IO_USER_BUF 0x0001 /* Don't deallocate buffer on close. */
#define _IO_UNBUFFERED 0x0002
#define _IO_NO_READS 0x0004 /* Reading not allowed.  */
#define _IO_NO_WRITES 0x0008 /* Writing not allowed.  */
#define _IO_EOF_SEEN 0x0010
#define _IO_ERR_SEEN 0x0020
#define _IO_DELETE_DONT_CLOSE 0x0040 /* Don't call close(_fileno) on close.  */
#define _IO_LINKED 0x0080 /* In the list of all open files.  */
#define _IO_IN_BACKUP 0x0100
#define _IO_LINE_BUF 0x0200
#define _IO_TIED_PUT_GET 0x0400 /* Put and get pointer move in unison.  */
#define _IO_CURRENTLY_PUTTING 0x0800
#define _IO_IS_APPENDING 0x1000
#define _IO_IS_FILEBUF 0x2000
/* 0x4000  No longer used, reserved for compat.  */
#define _IO_USER_LOCK 0x8000

struct __fbuf {
    char* base;
    char* ptr; /* current pointer */
    size_t size;
};

struct __rwbuf {
    __fbuf_t rbuf;
    __fbuf_t wbuf;
    char* base;
    size_t size;
};

struct __file {
    int _flags; /* flags, below; this FILE is free if 0 */
    int _file; /* fileno, if Unix descriptor, else -1 */
    size_t _r; /* read space left */
    size_t _w; /* write space left */
    __rwbuf_t _bf; /* rw buffer */
    int _ungotc; /* ungot char. If spot is empty, it equals to UNGOTC_EMPTY */
};

static FILE _stdstreams[3];
FILE* stdin = &_stdstreams[0];
FILE* stdout = &_stdstreams[1];
FILE* stderr = &_stdstreams[2];

/* Static functions */
static inline int _can_read(FILE* file);
static inline int _can_write(FILE* file);
static inline int _can_use_buffer(FILE* file);
static int _parse_mode(const char* mode, mode_t* flags);

/* Buffer */
static inline int _free_buf(FILE* stream);
static size_t _do_system_write(const void* ptr, size_t size, FILE* stream);
static int _resize_buf(FILE* stream, size_t size);
static ssize_t _flush_wbuf(FILE* stream);
static void _split_rwbuf(FILE* stream);
static int _resize_buf(FILE* stream, size_t size);

/* Stream */
static int _init_stream(FILE* file);
static int _init_file_with_fd(FILE* file, int fd);
static int _open_file(FILE* file, const char* path, const char* mode);
static FILE* _fopen_internal(const char* path, const char* mode);

/* Read/write */
static size_t _do_system_read(char* ptr, size_t size, FILE* stream);
static size_t _do_system_write(const void* ptr, size_t size, FILE* stream);
static size_t _fread_internal(char* ptr, size_t size, FILE* stream);
static size_t _fwrite_internal(const void* ptr, size_t size, FILE* stream);

/* Public functions */

FILE* fopen(const char* path, const char* mode)
{
    if (!path || !mode)
        return NULL;

    return _fopen_internal(path, mode);
}

size_t fread(void* ptr, size_t size, size_t count, FILE* stream)
{
    if (!ptr || !stream)
        return 0;

    return _fread_internal(ptr, size * count, stream);
}

size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)
{
    if (!ptr) {
        set_errno(EINVAL);
        return 0;
    }

    if (!stream) {
        set_errno(EINVAL);
        return 0;
    }

    return _fwrite_internal(ptr, size * count, stream);
}

/* TODO: Implement fseek */

int fputc(int c, FILE* stream)
{
    int res = fwrite(&c, 1, 1, stream);
    if (!res)
        return -errno;

    return c;
}

int putc(int c, FILE* stream)
{
    return fputc(c, stream);
}

int putchar(int c)
{
    return fputc(c, stdout);
}

int fputs(const char* s, FILE* stream)
{
    // HERE
    size_t len = strlen(s);

    int res = fwrite(s, len, 1, stream);

    if (!res)
        return -errno;

    return res;
}

int puts(const char* s)
{
    return fputs(s, stdout);
}

int fgetc(FILE* stream)
{
    char c;

    if (fread(&c, 1, 1, stream) != 1)
        return EOF;

    return c;
}

int getc(FILE* stream)
{
    return fgetc(stream);
}

int getchar()
{
    return fgetc(stdin);
}

int ungetc(int c, FILE* stream)
{
    if (c == EOF)
        return EOF;

    if (!stream) {
        set_errno(EINVAL);
        return EOF;
    }

    if (stream->_ungotc != UNGOTC_EMPTY) {
        set_errno(EBUSY);
        return EOF;
    }

    stream->_ungotc = c;
    return c;
}

char* fgets(char* s, int size, FILE* stream)
{
    unsigned int rd = 0;
    char c;

    if (!stream) {
        set_errno(EINVAL);
        return NULL;
    }

    /* We need to flush the stdout and stderr streams before reading. */
    fflush(stdout);
    fflush(stderr);

    while (c != '\n' && rd < size) {
        if ((c = fgetc(stream)) == EOF)
            return NULL;
        s[rd++] = c;
    }

    return s;
}

int setvbuf(FILE* stream, char* buf, int mode, size_t size)
{
    if (!stream) {
        set_errno(EINVAL);
        return -1;
    }

    if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF) {
        set_errno(EINVAL);
        return -1;
    }

    /* Clear the buffer type flags and reset it. */

    stream->_flags &= ~(int)(_IO_UNBUFFERED | _IO_LINE_BUF);
    if (mode & _IOLBF)
        stream->_flags |= _IO_LINE_BUF;

    if (mode & _IONBF)
        stream->_flags |= _IO_UNBUFFERED;

    _flush_wbuf(stream);

    if (!_can_use_buffer(stream))
        return _free_buf(stream);

    if (!buf)
        return _resize_buf(stream, size);

    stream->_bf.base = buf;
    stream->_bf.size = size;
    _split_rwbuf(stream);

    return 0;
}

void setbuf(FILE* stream, char* buf)
{
    setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}

void setlinebuf(FILE* stream)
{
    setvbuf(stream, NULL, _IOLBF, 0);
}

int fflush(FILE* stream)
{
    if (!stream)
        return -EBADF;

    return _flush_wbuf(stream);
}

int __stream_info(FILE *stream)
{
    static const char *names[] = {"(STDIN) ", "(STDOUT) ", "(STDERR) "};
    char rwinfo[4] = "-/-";
    __fbuf_t *rbuf, *wbuf;
    const char *name;

    if (!stream)
        return 1;

    if (stream->_file >= 0 && stream->_file <= 2)
        name = names[stream->_file];

    if (_can_read(stream)) {
        rwinfo[0] = 'r';
        rbuf = &stream->_bf.rbuf;
    }

    if (_can_write(stream)) {
        rwinfo[2] = 'w';
        wbuf = &stream->_bf.wbuf;
    }

    printf("__stream_info():\n");
    printf("  fd=%d %sflags=%s\n", stream->_file, name, rwinfo);
    printf("  ungotc=%s val=%x\n", stream->_ungotc == UNGOTC_EMPTY ? "False"
        : "True", stream->_ungotc);

    if (_can_read(stream)) {
        printf("  read space left=%u\n", stream->_r);
        printf("  rbuf.base=%x rbuf.size=%u rbuf.ptr=%x\n", (size_t) rbuf->base,
            rbuf->size, (size_t) rbuf->ptr);
    }

    if (_can_write(stream)) {
        printf("  write space left=%u\n", stream->_w);
        printf("  wbuf.base=%x wbuf.size=%u wbuf.ptr=%x\n", (size_t) wbuf->base,
            wbuf->size, (size_t) wbuf->ptr);
    }

    printf("  rwbuf.base=%x rwbuf.size=%u\n", (size_t) stream->_bf.base,
        stream->_bf.size);

    return 0;
}

int _stdio_init()
{
    _init_file_with_fd(stdin, STDIN);
    _init_file_with_fd(stdout, STDOUT);
    _init_file_with_fd(stderr, STDERR);
    setbuf(stderr, NULL);
    return 0;
}

int _stdio_deinit()
{
    // FIXME
    _flush_wbuf(stdout);
    return 0;
}

/* Static functions */

static inline int _can_read(FILE* file)
{
    return (file->_flags & _IO_NO_READS) == 0;
}

static inline int _can_write(FILE* file)
{
    return (file->_flags & _IO_NO_WRITES) == 0;
}

static inline int _can_use_buffer(FILE* file)
{
    return (file->_flags & _IO_UNBUFFERED) == 0;
}

/* Because this checks the first and second character only, the possible
   combinations are: r, w, a, r+ and w+. */
static int _parse_mode(const char* mode, mode_t* flags)
{
    int has_plus, len;

    if (!(len = strlen(mode)))
        return 0;

    *flags = 0;
    if (len > 1 && mode[1] == '+')
        has_plus = 1;

    switch (mode[0]) {
    case 'r':
        *flags = has_plus ? O_RDWR : O_RDONLY;
        return 0;

    case 'w':
        *flags = has_plus ? O_RDWR | O_CREAT : O_WRONLY | O_CREAT;
        return 0;

    case 'a':
        *flags = O_APPEND | O_CREAT;
        return 0;

        /* TODO: Add binary mode when the rest will support such option. */

    default:
        return -1;
    }

    return -1;
}

/* Buffer */

static inline int _free_buf(FILE* stream)
{
    /* Don't free the buffer if the user provided one with setvbuf. */
    if (stream->_flags & _IO_USER_BUF)
        return 0;

    if (stream->_bf.base)
        free(stream->_bf.base);

    return 0;
}

static void _split_rwbuf(FILE* stream)
{
    size_t rsize, wsize;

    rsize = ((stream->_bf.size + 1) / 2) & (size_t)~0x03;
    wsize = (stream->_bf.size - rsize) & (size_t)~0x03;

    /* TODO: Base on stream flags. */
    stream->_bf.rbuf.base = stream->_bf.base;
    stream->_bf.rbuf.ptr = stream->_bf.rbuf.base;
    stream->_bf.rbuf.size = rsize;

    stream->_bf.wbuf.base = stream->_bf.base + stream->_bf.rbuf.size;
    stream->_bf.wbuf.ptr = stream->_bf.wbuf.base;
    stream->_bf.wbuf.size = wsize;

    stream->_r = 0;
    stream->_w = stream->_bf.wbuf.size;
}

static int _resize_buf(FILE* stream, size_t size)
{
    _free_buf(stream);

    if (!size)
        return 0;

    stream->_bf.base = malloc(size);

    if (!stream->_bf.base) {
        stream->_r = 0;
        stream->_w = 0;
        return -1;
    }

    stream->_bf.size = size;
    _split_rwbuf(stream);

    return 0;
}

static ssize_t _flush_wbuf(FILE* stream)
{
    size_t write_size, written;

    write_size = stream->_bf.wbuf.size - stream->_w;
    written = _do_system_write(stream->_bf.wbuf.base, write_size, stream);

    if (written != write_size)
        return -EFAULT;

    stream->_w = stream->_bf.wbuf.size;
    stream->_bf.wbuf.ptr = stream->_bf.wbuf.base;
    return (ssize_t)write;
}

/* Stream */

static int _init_stream(FILE* file)
{
    file->_file = -1;
    file->_flags = _IO_MAGIC;
    file->_r = 0;
    file->_w = 0;
    file->_bf.base = NULL;
    file->_bf.size = 0;
    file->_ungotc = UNGOTC_EMPTY;
    return 0;
}

static int _init_file_with_fd(FILE* file, int fd)
{
    _init_stream(file);
    _resize_buf(file, BUFSIZ);
    file->_file = fd;
    return 0;
}

static int _open_file(FILE* file, const char* path, const char* mode)
{
    mode_t flags = 0;
    int err = _parse_mode(mode, &flags);

    if (err)
        return err;

    int fd = open(path, flags);
    if (fd < 0)
        return errno;

    file->_file = fd;
    return 0;
}

static FILE* _fopen_internal(const char* path, const char* mode)
{
    FILE* file = malloc(sizeof(FILE));
    if (!file)
        return NULL;

    _init_stream(file);
    _resize_buf(file, BUFSIZ);
    _open_file(file, path, mode);
    return file;
}

/* Read */

static size_t _do_system_read(char* ptr, size_t size, FILE* stream)
{
    ssize_t read_size = read(stream->_file, ptr, size);
    return read_size < 0 ? 0 : (size_t)read_size;
}

static size_t _do_system_write(const void* ptr, size_t size, FILE* stream)
{
    ssize_t write_size = write(stream->_file, ptr, size);

    if (write_size < 0)
        return 0;

    return (size_t)write_size;
}

static size_t _fread_internal(char* ptr, size_t size, FILE* stream)
{
    size_t total_size, read_from_buf;

    if (!size)
        return 0;

    if (!_can_use_buffer(stream))
        return _do_system_read(ptr, size, stream);

    total_size = 0;

    /* If the ungot char buffer is not empty, push it onto the buffer first. */
    if (stream->_ungotc != UNGOTC_EMPTY) {
        ptr[0] = (char)stream->_ungotc;
        ptr++;
        size--;
        total_size++;
        stream->_ungotc = UNGOTC_EMPTY;
    }

    /* First read any bytes still sitting in the read buffer. */
    if (stream->_r) {
        read_from_buf = min(stream->_r, size);
        memcpy(ptr, stream->_bf.rbuf.ptr, read_from_buf);
        ptr += read_from_buf;
        size -= read_from_buf;
        stream->_bf.rbuf.ptr += read_from_buf;
        stream->_r -= read_from_buf;
        total_size += read_from_buf;
    }

    /* Read the remaining bytes that were not stored in the read buffer. */
    while (size > 0) {
        stream->_bf.rbuf.ptr = stream->_bf.rbuf.base;
        stream->_r = _do_system_read(
            stream->_bf.rbuf.ptr, stream->_bf.rbuf.size, stream);

        if (!stream->_r)
            return total_size;

        read_from_buf = min(stream->_r, size);
        memcpy(ptr, stream->_bf.rbuf.ptr, read_from_buf);
        ptr += read_from_buf;
        size -= read_from_buf;
        stream->_bf.rbuf.ptr += read_from_buf;
        stream->_r -= read_from_buf;
        total_size += read_from_buf;
    }

    return total_size;
}

static size_t _fwrite_internal(const void* ptr, size_t size, FILE* stream)
{
    size_t total_size;

    if (!_can_use_buffer(stream))
        return _do_system_write(ptr, size, stream);

    total_size = 0;
    while (size > 0) {
        size_t write_size = min(stream->_w, size);
        memcpy(stream->_bf.wbuf.ptr, ptr, write_size);
        ptr += write_size;
        size -= write_size;
        stream->_bf.wbuf.ptr += write_size;
        stream->_w -= write_size;
        total_size += write_size;

        if (!stream->_w)
            _flush_wbuf(stream);
    }

    return total_size;
}
