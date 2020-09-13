#include <drivers/display.h>
#include <drivers/keyboard.h>
#include <errno.h>
#include <fs/devfs/devfs.h>
#include <fs/vfs.h>
#include <io/tty/tty.h>
#include <mem/kmalloc.h>
#include <syscall_structs.h>
#include <tasking/signal.h>
#include <tasking/tasking.h>

static int next_tty = 0;
static tty_entry_t* active_tty = 0;
tty_entry_t ttys[TTY_MAX_COUNT];

static tty_entry_t* _tty_get(dentry_t* dentry)
{
    for (int i = 0; i < TTY_MAX_COUNT; i++) {
        if (dentry->inode_indx == ttys[i].inode_indx) {
            return &ttys[i];
        }
    }
    return 0;
}

inline static tty_entry_t* _tty_active()
{
    return active_tty;
}

bool tty_can_read(dentry_t* dentry)
{
    return _tty_get(dentry)->lines_avail > 0;
}

bool tty_can_write(dentry_t* dentry)
{
    return true;
}

int tty_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    tty_entry_t* tty = _tty_get(dentry);
    if (tty->lines_avail <= 0) {
        return -1;
    }
    uint32_t leno = ringbuffer_space_to_read(&tty->buffer);
    int res = ringbuffer_read(&tty->buffer, buf, leno);
    tty->lines_avail--;
    return leno - 1; // the end is null-termination sign, we don't count
}

int tty_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    kprintf("%s", buf);
    return len;
}

int tty_ioctl(dentry_t* dentry, uint32_t cmd, uint32_t arg)
{
    tty_entry_t* tty = _tty_get(dentry);

    switch (cmd) {
    case TIOCGPGRP:
        return tty->pgid;
    case TIOCSPGRP:
        tty->pgid = arg;
        return 0;
    }

    return -EINVAL;
}

tty_entry_t* tty_new()
{
    dentry_t* mp;
    if (vfs_resolve_path("/dev", &mp) < 0) {
        return 0;
    }

    char* name = "tty ";
    name[3] = next_tty + '0';
    file_ops_t fops;
    fops.can_read = tty_can_read;
    fops.can_write = tty_can_write;
    fops.read = tty_read;
    fops.write = tty_write;
    fops.ioctl = tty_ioctl;
    devfs_inode_t* res = devfs_register(mp, name, 4, 0, &fops);
    ttys[next_tty].id = next_tty;
    ttys[next_tty].inode_indx = res->index;
    ttys[next_tty].buffer = ringbuffer_create_std();
    ttys[next_tty].lines_avail = 0;
    if (!ttys[next_tty].buffer.zone.start) {
        kprintf("Error: tty buffer allocation");
        while (1) { }
    }
    active_tty = &ttys[next_tty];
    next_tty++;

    dentry_put(mp);
    return &ttys[next_tty - 1];
}

void tty_eat_key(key_t key)
{
    tty_entry_t* tty = _tty_active();
    if (key == KEY_CTRLC) {
        proc_t* p = tasking_get_proc(tty->pgid);
        if (p) {
            signal_set_pending(p->main_thread, SIGINT);
            signal_dispatch_pending(p->main_thread);
        }
        return;
    }

    if (key == KEY_RETURN) {
        print_char('\n', WHITE_ON_BLACK, -1, -1);
        ringbuffer_write_one(&tty->buffer, '\n');
        ringbuffer_write_one(&tty->buffer, '\0');
        tty->lines_avail++;
    } else if (key == KEY_BACKSPACE) {
        if (ringbuffer_space_to_read(&tty->buffer) > 0) {
            delete_char(WHITE_ON_BLACK, -1, -1, 1);
            tty->buffer.end--;
        }
    } else {
        ringbuffer_write_one(&tty->buffer, (char)key);
        print_char(key, WHITE_ON_BLACK, -1, -1);
    }
}