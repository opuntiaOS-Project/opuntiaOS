#include <dirent/dirent.h>
#include <errno.h>
#include <syscalls.h>

/* TODO: Will be replaced with malloc */
DIR dirarr[8];
int nxt = 0;

DIR* opendir(char* name)
{
    int fd = open(name, O_RDONLY | O_DIRECTORY);
    if (fd < 0) {
        errno = -fd;
        return 0;
    }

    /* TODO: Replace with malloc */
    dirarr[nxt].fd = fd;
    dirarr[nxt].size = 128;
    dirarr[nxt].allocated = 128;
    dirarr[nxt].offset = 0;
    return &dirarr[nxt++];
}

int closedir(DIR* dir)
{
    if (dir->fd < 0) {
        errno = EBADF;
        return -1;
    }

    // close(dir->fd);
    return 0;
}

/* TODO: Rewrite when malloc is available */
// dirent_t readdir(DIR* dir)
// {
// }