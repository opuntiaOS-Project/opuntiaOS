#include <tty/tty.h>
#include <fs/vfs.h>
#include <fs/devfs/devfs.h>
#include <drivers/display.h>
#include <drivers/keyboard.h>

char ttybuf[1024];
int bufstart = 0;
int bufend = 0;
int lines_avail = 0;

bool tty_can_read()
{
    return lines_avail != 0;
}

int tty_read(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    int leno = bufend - bufstart;
    memcpy(buf, ttybuf+bufstart, leno);
    bufstart = bufend;
    lines_avail--;
    return leno;
}

int tty_write(dentry_t* dentry, uint8_t* buf, uint32_t start, uint32_t len)
{
    kprintf("%s", buf);
    return 0;
}

void tty_setup()
{
    clean_screen();
    dentry_t* mp;
    if (vfs_resolve_path("/dev", &mp) < 0) {
        return;
    }
    
    file_ops_t fops;
    fops.can_read = tty_can_read;
    fops.read = tty_read;
    fops.write = tty_write;
    devfs_register(mp, "tty0", 4, 0, &fops);

    dentry_put(mp);
}

void tty_eat_key(char key)
{
    if (key == KEY_RETURN) {
        print_char('\n', WHITE_ON_BLACK, -1, -1);
        ttybuf[bufend++] = '\n';
        ttybuf[bufend++] = '\0';
        lines_avail++;
    } else {
        ttybuf[bufend++] = key;
        print_char(key, WHITE_ON_BLACK, -1, -1);
    }
}