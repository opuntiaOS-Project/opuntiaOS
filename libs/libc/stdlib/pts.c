#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#define _PATH_PTS "/dev/pts"
#define MASTER_PTY(dev) (major(dev) == 128)
#define SLAVE_PTY(dev) (major(dev) == 136)

int posix_openpt(int flags)
{
    return open("/dev/ptmx", flags);
}

int ptsname_r(int fd, char* buf, size_t buflen)
{
    int len = strlen(_PATH_PTS);
    if (buflen < len + 2) {
        set_errno(ERANGE);
        return -ERANGE;
    }

    fstat_t stat;
    if (fstat(fd, &stat) < 0) {
        return errno;
    }

    if (!MASTER_PTY(stat.dev)) {
        set_errno(ENOTTY);
        return -ENOTTY;
    }

    int ptyno = minor(stat.dev);

    char* p = strcpy(buf, _PATH_PTS);
    p[len + 0] = '0' + ptyno;
    p[len + 1] = '\0';

    return 0;
}

static char ptsbuf[32];
char* ptsname(int fd)
{
    if (ptsname_r(fd, ptsbuf, sizeof(ptsbuf)) < 0) {
        return NULL;
    }
    return ptsbuf;
}
