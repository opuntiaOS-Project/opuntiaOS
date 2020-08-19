#include <drivers/display.h>
#include <drivers/keyboard.h>
#include <fs/devfs/devfs.h>
#include <mem/kmalloc.h>
#include <fs/vfs.h>
#include <tty/tty.h>

static int next_tty = 0;
static tty_entry_t* active_tty = 0;
tty_entry_t ttys[TTY_MAX_COUNT];

static tty_entry_t* _tty_get(dentry_t* dentry)
{
    for (int i = 0; i < TTY_MAX_COUNT; i++) {
        if (dentry->inode_indx == ttys[i].inode_indx){
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
    return _tty_get(dentry)->lines_avail != 0;
}

int tty_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    tty_entry_t* tty = _tty_get(dentry);
    int leno = tty->end - tty->start;
    memcpy(buf, tty->buffer + tty->start, leno);
    tty->start = tty->end;
    tty->lines_avail--;
    return leno;
}

int tty_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    kprintf("%s", buf);
    return 0;
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
    fops.read = tty_read;
    fops.write = tty_write;
    devfs_inode_t* res = devfs_register(mp, name, 4, 0, &fops);
    ttys[next_tty].id = next_tty;
    ttys[next_tty].inode_indx = res->index;
    ttys[next_tty].start = 0;
    ttys[next_tty].end = 0;
    ttys[next_tty].lines_avail = 0;
    ttys[next_tty].buffer = (char*)kmalloc(TTY_BUFFER_SIZE);
    if (!ttys[next_tty].buffer) {
        kprintf("Error: tty malloc");
        while(1) {}
    }
    active_tty = &ttys[next_tty];
    next_tty++;

    dentry_put(mp);
    return &ttys[next_tty-1];
}

void tty_eat_key(char key)
{
    tty_entry_t* tty = _tty_active();
    if (key == KEY_RETURN) {
        print_char('\n', WHITE_ON_BLACK, -1, -1);
        tty->buffer[tty->end++] = '\n';
        tty->buffer[tty->end++] = '\0';
        tty->lines_avail++;
    } else if (key == KEY_BACKSPACE) {
        if (tty->end > tty->start) {
            delete_char(WHITE_ON_BLACK, -1, -1, 1);
            tty->end--;
        }
    } else {
        tty->buffer[tty->end++] = key;
        print_char(key, WHITE_ON_BLACK, -1, -1);
    }
}