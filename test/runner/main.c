#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUF_SIZE 4096
char buf[BUF_SIZE];
char namebuf[256];

struct linux_dirent {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char* name;
};

void test_failed(const char* reason)
{
    printf("[FAILED] %s\n", reason);
    fflush(stdout);
    exit(1);
}

void test_ok(const char* name)
{
    printf("[OK] %s\n", name);
    fflush(stdout);
}

int main(int argc, char** argv)
{
    int fd, nread;
    struct linux_dirent* d;
    int bpos;

    fd = open("/test_bin", O_RDONLY | O_DIRECTORY);
    if (fd < 0) {
        test_failed("can't open /test_bin\n");
    }

    for (;;) {
        nread = getdents(fd, buf, BUF_SIZE);
        if (nread < 0) {
            test_failed("can't read dir");
        }

        if (nread == 0)
            break;

        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent*)(buf + bpos);
            if (((char*)&d->name)[0] != '.') {
                int pid = fork();
                if (pid) {
                    int out = 0;
                    waitpid(pid, &out);
                    if (out != 0) {
                        test_failed((char*)&d->name);
                    } else {
                        test_ok((char*)&d->name);
                    }
                } else {
                    setuid(10);
                    setgid(10);
                    sprintf(namebuf, "/test_bin/%s", (char*)&d->name);
                    execve(namebuf, 0, 0);
                    exit(0);
                }
            }
            bpos += d->rec_len;
        }
    }

    printf("[ALL TESTS PASSED]\n\n");
    fflush(stdout);
    return 0;
}