#ifndef _LIBC_SYS_SHARED_BUFFER_H
#define _LIBC_SYS_SHARED_BUFFER_H

#include <bits/sys/select.h>
#include <bits/time.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int shared_buffer_create(uint8_t** buffer, size_t size);
int shared_buffer_get(int id, uint8_t** buffer);
int shared_buffer_free(int id);

__END_DECLS

#endif // _LIBC_SYS_SHARED_BUFFER_H