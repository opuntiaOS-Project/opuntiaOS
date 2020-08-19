#ifndef __oneOS__TTY__TTY_H
#define __oneOS__TTY__TTY_H

#define TTY_MAX_COUNT 8
#define TTY_BUFFER_SIZE 1024

struct tty_entry {
    int id;
    int inode_indx;
    char* buffer;
    int start;
    int end;
    int lines_avail;
};
typedef struct tty_entry tty_entry_t;

extern tty_entry_t ttys[TTY_MAX_COUNT];

tty_entry_t* tty_new();
void tty_eat_key(char key);

#endif // __oneOS__TTY__TTY_H