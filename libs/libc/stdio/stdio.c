#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_structs.h>
#include <syscalls.h>
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

static FILE _stdstreams[3];
FILE* stdin = &_stdstreams[0];
FILE* stdout = &_stdstreams[1];
FILE* stderr = &_stdstreams[2];

static size_t _do_system_write(const void* ptr, size_t size, FILE* stream);

static int _resize_buf(FILE* stream, size_t size);
static ssize_t _flush_wbuf(FILE* stream);

/**
 * FLAGS
 */

inline static int _can_read(FILE* file)
{
    return ((file->_flags & _IO_NO_READS) == 0);
}

inline static int _can_write(FILE* file)
{
    return ((file->_flags & _IO_NO_WRITES) == 0);
}

inline static int _can_use_buffer(FILE* file)
{
    return ((file->_flags & _IO_UNBUFFERED) == 0);
}

/**
 * BUFFER
 */

static inline void _init_buf(FILE* stream)
{
    _resize_buf(stream, BUFSIZ);
}

static inline int _free_buf(FILE* stream)
{
    if (!(stream->_flags & _IO_USER_BUF) && stream->_bf.base) {
        free(stream->_bf.base);
    }
    return 0;
}

/**
 * _split_rwbuf splits newly allocated buffer to a read buffer
 * and a write buffer.
 */
static void _split_rwbuf(FILE* stream)
{
    // TODO: Base on stream flags.
    stream->_bf.rbuf.base = stream->_bf.base;
    stream->_bf.rbuf.ptr = stream->_bf.rbuf.base;
    stream->_bf.rbuf.size = ((stream->_bf.size + 1) / 2) & ((size_t)~(0b11));

    stream->_bf.wbuf.base = stream->_bf.base + stream->_bf.rbuf.size;
    stream->_bf.wbuf.ptr = stream->_bf.wbuf.base;
    stream->_bf.wbuf.size = (stream->_bf.size - stream->_bf.rbuf.size) & ((size_t)~(0b11));

    stream->_r = 0;
    stream->_w = stream->_bf.wbuf.size;
}

/**
 * _resize_buf frees prev buffer and allocates new with size.
 */
static int _resize_buf(FILE* stream, size_t size)
{
    _free_buf(stream);

    if (!size) {
        return 0;
    }

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

static int _set_buf(FILE* stream, char* buf, int mode, size_t size)
{
    stream->_flags &= ~(int)(_IO_UNBUFFERED | _IO_LINE_BUF);
    if (mode & _IOLBF) {
        stream->_flags |= _IO_LINE_BUF;
    }
    if (mode & _IONBF) {
        stream->_flags |= _IO_UNBUFFERED;
    }

    _flush_wbuf(stream);

    if (!_can_use_buffer(stream)) {
        return _free_buf(stream);
    }

    if (!buf) {
        return _resize_buf(stream, size);
    }

    stream->_bf.base = buf;
    stream->_bf.size = size;
    _split_rwbuf(stream);
    return 0;
}

static ssize_t _flush_wbuf(FILE* stream)
{
    size_t write_size = stream->_bf.wbuf.size - stream->_w;
    size_t written = _do_system_write(stream->_bf.wbuf.base, write_size, stream);
    if (written != write_size) {
        return -EFAULT;
    }
    stream->_w = stream->_bf.wbuf.size;
    stream->_bf.wbuf.ptr = stream->_bf.wbuf.base;
    return (ssize_t)write;
}

static int _parse_mode(const char* mode, mode_t* flags)
{
    *flags = 0;
    int len = strlen(mode);
    if (!len) {
        return 0;
    }
    // TODO: Add binary mode
    int has_plus = 0;
    for (int i = 0; i < len; i++) {
        if (mode[i] == '+') {
            has_plus = 1;
        }
    }

    switch (mode[0]) {
    case 'r':
        if (has_plus) {
            *flags = O_RDWR;
        } else {
            *flags = O_RDONLY;
        }
        return 0;

    case 'w':
        if (has_plus) {
            *flags = O_RDWR | O_CREATE;
        } else {
            *flags = O_WRONLY | O_CREATE;
        }
        return 0;

    case 'a':
        if (has_plus) {
            *flags = O_APPEND | O_CREATE;
        } else {
            *flags = O_APPEND | O_CREATE;
        }
        return 0;

    default:
        return -1;
    }

    return -1;
}

/**
 * STREAM
 */

static int _init_stream(FILE* file)
{
    file->_file = -1;
    file->_flags = _IO_MAGIC;
    file->_r = 0;
    file->_w = 0;
    file->_bf.base = NULL;
    file->_bf.size = 0;
    return 0;
}

static int _init_file_with_fd(FILE* file, int fd)
{
    _init_stream(file);
    _init_buf(file);
    file->_file = fd;
    return 0;
}

static int _open_file(FILE* file, const char* path, const char* mode)
{
    mode_t flags = 0;
    int err = _parse_mode(mode, &flags);
    if (err) {
        return err;
    }

    int fd = open(path, flags);
    if (fd < 0) {
        return errno;
    }

    file->_file = fd;
}

static FILE* _fopen_internal(const char* path, const char* mode)
{
    FILE* file = malloc(sizeof(FILE));
    if (!file) {
        return NULL;
    }

    _init_stream(file);
    _init_buf(file);
    _open_file(file, path, mode);
    return file;
}

FILE* fopen(const char* path, const char* mode)
{
    if (!path) {
        return NULL;
    }
    if (!mode) {
        return NULL;
    }
    return _fopen_internal(path, mode);
}

static size_t _do_system_read(void* ptr, size_t size, FILE* stream)
{
    ssize_t read_cnt = read(stream->_file, ptr, size);
    if (read_cnt < 0) {
        return 0;
    }
    return (size_t)read_cnt;
}

static size_t _fread_internal(void* ptr, size_t size, FILE* stream)
{
    if (!_can_use_buffer(stream)) {
        return _do_system_read(ptr, size, stream);
    }

    size_t total_size = 0;

    if (stream->_r) {
        size_t read_from_buf = min(stream->_r, size);
        memcpy(ptr, stream->_bf.rbuf.ptr, read_from_buf);
        ptr += read_from_buf;
        size -= read_from_buf;
        stream->_bf.rbuf.ptr += read_from_buf;
        stream->_r -= read_from_buf;
        total_size += read_from_buf;
    }

    while (size > 0) {
        stream->_bf.rbuf.ptr = stream->_bf.rbuf.base;
        stream->_r = _do_system_read(stream->_bf.rbuf.ptr, stream->_bf.rbuf.size, stream);

        if (!stream->_r) {
            return total_size;
        }

        size_t read_from_buf = min(stream->_r, size);
        memcpy(ptr, stream->_bf.rbuf.ptr, read_from_buf);
        ptr += read_from_buf;
        size -= read_from_buf;
        stream->_bf.rbuf.ptr += read_from_buf;
        stream->_r -= read_from_buf;
        total_size += read_from_buf;
    }

    return total_size;
}

size_t fread(void* ptr, size_t size, size_t count, FILE* stream)
{
    if (!ptr) {
        return 0;
    }
    if (!stream) {
        return 0;
    }
    return _fread_internal(ptr, size * count, stream);
}

static size_t _do_system_write(const void* ptr, size_t size, FILE* stream)
{
    ssize_t write_cnt = write(stream->_file, ptr, size);
    if (write_cnt < 0) {
        return 0;
    }
    return (size_t)write_cnt;
}

static size_t _fwrite_internal(const void* ptr, size_t size, FILE* stream)
{
    if (!_can_use_buffer(stream)) {
        return _do_system_write(ptr, size, stream);
    }

    size_t total_size = 0;

    while (size > 0) {
        size_t write_size = min(stream->_w, size);
        memcpy(stream->_bf.wbuf.ptr, ptr, write_size);
        ptr += write_size;
        size -= write_size;
        stream->_bf.wbuf.ptr += write_size;
        stream->_w -= write_size;
        total_size += write_size;
        if (!stream->_w) {
            _flush_wbuf(stream);
        }
    }

    return total_size;
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

int fseek(FILE* stream, uint32_t offset, int whence)
{
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

    return _set_buf(stream, buf, mode, size);
}

void setbuf(FILE* stream, char* buf)
{
    setvbuf(stream, buf, buf ? _IOFBF : _IONBF, BUFSIZ);
}

void setlinebuf(FILE* stream)
{
    setvbuf(stream, NULL, _IOLBF, 0);
}

int _stdio_init()
{
    _init_file_with_fd(stdin, 0);
    _init_file_with_fd(stdout, 1);
    _init_file_with_fd(stderr, 2);
    setbuf(stderr, NULL);
    return 0;
}

int _stdio_deinit()
{
    // FIXME
    _flush_wbuf(stdout);
    return 0;
}