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
#include <utils.h>
#include <log.h>

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

static inline void _tty_flush_input(tty_entry_t* tty)
{
    tty->lines_avail = 0;
    ringbuffer_clear(&tty->buffer);
}

inline static tty_entry_t* _tty_active()
{
    return active_tty;
}

bool tty_can_read(dentry_t* dentry)
{
    tty_entry_t* tty = _tty_get(dentry);
    if (tty->termios.c_lflag & ICANON) {
        return tty->lines_avail > 0;
    }
    return ringbuffer_space_to_read(&tty->buffer) >= 1;
}

bool tty_can_write(dentry_t* dentry)
{
    return true;
}

int tty_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    tty_entry_t* tty = _tty_get(dentry);
    uint32_t leno = ringbuffer_space_to_read(&tty->buffer);
    if (leno > len) {
        leno = len;
    }
    int res = ringbuffer_read(&tty->buffer, buf, leno);
    if (tty->termios.c_lflag & ICANON && res == leno) {
        tty->lines_avail--;
    }
    return leno;
}

int _tty_process_esc_seq(uint8_t* buf)
{
    if (buf[0] != '\x1b') {
        return 0;
    }
    int argv[4] = { 0, 0, 0, 0 };
    int id = 1;
    int argc = 0;

    if (buf[id] == '[') {
        id++;
        for (;;) {
            int len = 0;
            while ('0' <= buf[id + len] && buf[id +len] <= '9') {
                len++;
            }
            argv[argc++] = stoi(&buf[id], len);
            id += len;
            if (buf[id] != ';') {
                break;
            }
            id++;
        }
    }

    char cmd = buf[id];
    id++;
    switch (cmd) {
    case 'J':
        if (argv[0] == 2) {
            clean_screen();
        }
        return id;
    case 'H':
        if (argc == 0) {
            set_cursor_offset(get_offset(0, 0));
        }
        if (argc == 2) {
            set_cursor_offset(get_offset(argv[1] - 1, argv[0] - 1));
        }
        return id;
    }
}

int tty_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    for (int i = 0; i < len; i++) {
        if (buf[i] == '\x1b') {
            i += _tty_process_esc_seq(&buf[i]);
        } else {
            print_char(buf[i], WHITE_ON_BLACK, -1, -1);
        }
    }

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
    case TCGETS:
        memcpy((void*)arg, (void*)&tty->termios, sizeof(termios_t));
        return 0;
    case TCSETS:
    case TCSETSW:
    case TCSETSF:
        memcpy((void*)&tty->termios, (void*)arg, sizeof(termios_t));
        if (cmd == TCSETSF) {
            _tty_flush_input(tty);
        }
        return 0;
    }

    return -EINVAL;
}

static void _tty_setup_termios(tty_entry_t* tty)
{
    tty->termios.c_lflag |= ECHO | ICANON;
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
    _tty_setup_termios(&ttys[next_tty]);
    if (!ttys[next_tty].buffer.zone.start) {
        kprintf("Error: tty buffer allocation");
        while (1) { }
    }
    active_tty = &ttys[next_tty];
    next_tty++;

    dentry_put(mp);
    return &ttys[next_tty - 1];
}

static void _tty_echo_key(tty_entry_t* tty, key_t key)
{
    if (tty->termios.c_lflag & ECHO) {
        print_char(key, WHITE_ON_BLACK, -1, -1);
    }
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
        _tty_echo_key(tty, '\n');
        ringbuffer_write_one(&tty->buffer, '\n');
        tty->lines_avail++;
    } else if (key == KEY_BACKSPACE) {
        if (ringbuffer_space_to_read(&tty->buffer) > 0) {
            delete_char(WHITE_ON_BLACK, -1, -1, 1);
            tty->buffer.end--;
        }
    } else {
        ringbuffer_write_one(&tty->buffer, (char)key);
        _tty_echo_key(tty, key);
    }
}