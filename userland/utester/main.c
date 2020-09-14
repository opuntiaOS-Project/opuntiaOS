#include <libsystem/malloc.h>
#include <libsystem/syscalls.h>
#include <libsystem/string.h>

void exitwait(void)
{
    int i, pid;

    for (i = 0; i < 20; i++) {
        pid = fork();
        if (pid < 0) {
            write(1, "fork failed\n", 12);
            return;
        }
        if (pid) {
            wait(pid);
        } else {
            exit(0);
        }
    }
    write(1, "exitwait ok\n", 12);
}

void mem(void)
{
    void *m1, *m2;
    int pid, ppid;

    write(1, "mem test\n", 9);
    ppid = getpid();
    if ((pid = fork()) == 0) {
        m1 = 0;
        for (int i = 0; i < 1; i++) {
            m2 = malloc(10001);
            if (m2 == 0) {
                write(1, "couldn't allocate mem1!!\n", 25);
                kill(ppid, 9);
                exit(1);
            }
            *(char**)m2 = m1;
            m1 = m2;
        }
        while (m1) {
            m2 = *(char**)m1;
            free(m1);
            m1 = m2;
        }
        m1 = malloc(1024 * 20);
        if (m1 == 0) {
            write(1, "couldn't allocate mem?!!\n", 25);
            kill(ppid, 9);
            exit(1);
        }
        free(m1);
        write(1, "mem ok\n", 7);
        exit(0);
    } else {
        wait(pid);
    }
}

char buf[512];
// four processes write different files at the same
// time, to test block allocation.
void fourfiles(void)
{
    int fd, pid, i, j, n, total, pi;
    char* names[] = { "f0.e", "f1.e", "f2.e", "f3.e" };
    char* fname;
    int pids[4];

    write(1, "fourfiles test\n", 15);

    for (pi = 0; pi < 4; pi++) {
        fname = names[pi];
        unlink(fname);

        pid = fork();
        pids[pi] = pid;
        if (pid < 0) {
            write(1, "fork failed\n", 12);
            exit(-1);
        }

        if (pid == 0) {
            fd = open(fname, O_CREATE | O_RDWR);
            if (fd < 0) {
                write(1, "create failed\n", 14);
                exit(-1);
            }

            memset(buf, '0' + pi, 512);
            for (i = 0; i < 12; i++) {
                if ((n = write(fd, buf, 500)) != 500) {
                    write(1, "write failed\n", 14);
                    exit(-1);
                }
            }
            exit(-1);
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
                    write(1, "wrong char\n", 11);
                    exit(-1);
                }
            }
            total += n;
        }
        close(fd);
        if (total != 12 * 500) {
            write(1, "wrong length\n", 13);
            exit(-1);
        }
        unlink(fname);
    }

    write(1, "fourfiles ok\n", 13);
}

int main(int argc, char** argv)
{
    mem();
    exitwait();
    fourfiles();
    return 0;
}