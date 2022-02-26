#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

char buf[512];

int main(int argc, char** argv)
{
    int fd, pid, i, j, n, total, pi;
    char* names[] = { "f0.e", "f1.e", "f2.e", "f3.e" };
    char* fname;
    int pids[4];

    for (pi = 0; pi < 4; pi++) {
        fname = names[pi];
        unlink(fname);

        pid = fork();
        pids[pi] = pid;
        if (pid < 0) {
            TestErr("fork failed");
        }

        if (pid == 0) {
            fd = open(fname, O_CREAT | O_RDWR, 0600);
            if (fd < 0) {
                TestErr("create failed");
            }

            memset(buf, '0' + pi, 512);
            for (i = 0; i < 12; i++) {
                if ((n = write(fd, buf, 500)) != 500) {
                    TestErr("write failed");
                }
            }
            exit(0);
        }
    }

    for (pi = 0; pi < 4; pi++) {
        wait(pids[pi]);
    }

    for (i = 0; i < 4; i++) {
        fname = names[i];
        fd = open(fname, O_RDONLY);
        total = 0;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            for (j = 0; j < n; j++) {
                if (buf[j] != '0' + i) {
                    TestErr("wrong char");
                }
            }
            total += n;
        }
        close(fd);
        if (total != 12 * 500) {
            TestErr("wrong length");
        }
        unlink(fname);
    }

    return 0;
}