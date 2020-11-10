#ifndef __oneOS__TTY__PTY_SLAVE_H
#define __oneOS__TTY__PTY_SLAVE_H

#ifndef PTYS_COUNT
#define PTYS_COUNT 4
#endif

struct pty_master_entry;
struct pty_slave_entry {
    int inode_indx;
    struct pty_master_entry* ptm;
    ringbuffer_t buffer;
};
typedef struct pty_slave_entry pty_slave_entry_t;

extern pty_slave_entry_t pty_slaves[PTYS_COUNT];

int pty_slave_create(int id, struct pty_master_entry* ptm);

#endif