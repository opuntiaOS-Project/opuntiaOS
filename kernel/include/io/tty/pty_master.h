#ifndef _KERNEL_IO_TTY_PTY_MASTER_H
#define _KERNEL_IO_TTY_PTY_MASTER_H

#include <fs/vfs.h>

#ifndef PTYS_COUNT
#define PTYS_COUNT 4
#endif

struct pty_slave_entry;
struct pty_master_entry {
    ringbuffer_t buffer;
    struct pty_slave_entry* pts;
    dentry_t dentry;
};
typedef struct pty_master_entry pty_master_entry_t;

extern pty_master_entry_t pty_masters[PTYS_COUNT];

int pty_master_alloc(file_descriptor_t* fd);

#endif