#ifndef __oneOS__TTY__TTY_H
#define __oneOS__TTY__TTY_H

#include <algo/ringbuffer.h>
#include <drivers/keyboard.h>
#include <types.h>

#define TTY_MAX_COUNT 8
#define TTY_BUFFER_SIZE 1024

struct tty_entry {
    int id;
    int inode_indx;
    ringbuffer_t buffer;
    int lines_avail;
};
typedef struct tty_entry tty_entry_t;

extern tty_entry_t ttys[TTY_MAX_COUNT];

tty_entry_t* tty_new();
void tty_eat_key(key_t key);

#endif // __oneOS__TTY__TTY_H