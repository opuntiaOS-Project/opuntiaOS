#include <opuntia/shared_buffer.h>
#include <sysdep.h>

int shared_buffer_create(uint8_t** buffer, size_t size)
{
    int res = DO_SYSCALL_2(SYS_SHBUF_CREATE, buffer, size);
    RETURN_WITH_ERRNO(res, res, res);
}

int shared_buffer_get(int id, uint8_t** buffer)
{
    int res = DO_SYSCALL_2(SYS_SHBUF_GET, id, buffer);
    RETURN_WITH_ERRNO(res, res, res);
}

int shared_buffer_free(int id)
{
    int res = DO_SYSCALL_1(SYS_SHBUF_FREE, id);
    RETURN_WITH_ERRNO(res, res, res);
}
