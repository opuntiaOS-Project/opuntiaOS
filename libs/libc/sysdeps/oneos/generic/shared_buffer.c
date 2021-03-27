#include <sys/shared_buffer.h>
#include <sysdep.h>

int shared_buffer_create(uint8_t** buffer, size_t size)
{
    return DO_SYSCALL_2(SYS_SHBUF_CREATE, buffer, size);
}

int shared_buffer_get(int id, uint8_t** buffer)
{
    return DO_SYSCALL_2(SYS_SHBUF_GET, id, buffer);
}

int shared_buffer_free(int id)
{
    return DO_SYSCALL_1(SYS_SHBUF_FREE, id);
}
