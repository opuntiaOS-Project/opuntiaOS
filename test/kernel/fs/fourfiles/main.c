#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char buf[512];

void msg(const char* msg)
{
    printf("[MSG] %s\n", msg);
    fflush(stdout);
}

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
            msg("fork failed");
            return 1;
        }

        if (pid == 0) {
            fd = open(fname, O_CREAT | O_RDWR);
            if (fd < 0) {
                msg("create failed");
                return 1;
            }

            memset(buf, '0' + pi, 512);
            for (i = 0; i < 12; i++) {
                if ((n = write(fd, buf, 500)) != 500) {
                    msg("write failed");
                    return 1;
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
        fd = open(fname, 0);
        total = 0;
        while ((n = read(fd, buf, sizeof(buf))) > 0) {
            for (j = 0; j < n; j++) {
                if (buf[j] != '0' + i) {
                    msg("wrong char");
                    return 1;
                }
            }
            total += n;
        }
        close(fd);
        if (total != 12 * 500) {
            msg("wrong length");
            return 1;
        }
        unlink(fname);
    }

    return 0;
}