#include <fcntl.h>
#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void exectest(void)
{
    int i, pid;

    char* paramscat[] = {
        "../readme",
        "../readme",
        (char*)0,
    };

    for (i = 0; i < 20; i++) {
        pid = fork();
        if (pid < 0) {
            write(1, "fork failed\n", 12);
            return;
        }
        if (pid) {
            wait(pid);
        } else {
            execve("/bin/cat", paramscat, 0);
            write(1, "exec failed\n", 12);
            exit(0);
        }
    }
    write(1, "exectest ok\n", 12);
}

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

void dirfile(void)
{
    int fd;

    write(1, "dir vs file\n", 12);

    fd = open("dirfile", 0);
    if (fd >= 0) {
        write(1, "create dirfile succeeded!\n", 26);
        exit(-1);
    }
    fd = open("dirfile", O_CREATE);
    if (chdir("dirfile") == 0) {
        write(1, "chdir dirfile succeeded!\n", 25);
        exit(-1);
    }
    if (unlink("dirfile") != 0) {
        write(1, "unlink dirfile failed!\n", 23);
        exit(-1);
    }

    fd = open(".", O_RDWR);
    if (fd >= 0) {
        write(1, "open . for writing succeeded!\n", 30);
        exit(-1);
    }
    fd = open(".", 0);
    if (write(fd, "x", 1) > 0) {
        write(1, "write . succeeded!\n", 19);
        exit(-1);
    }
    close(fd);

    write(1, "dir vs file OK\n", 15);
}

static volatile int rev = 0;

int inter(int no)
{
    write(1, ")", 1);
    rev++;
    return 0;
}

void testsignals()
{
    write(1, "signals test\n", 13);
    int pid = fork();
    if (pid) {
        sched_yield();
        for (int i = 0; i < 50; i++) {
            write(1, "(", 1);
            kill(pid, 3);
            sched_yield();
        }

        wait(pid);
    } else {
        sigaction(3, inter);
        while (rev != 50) {
        }

        for (int i = 0; i < 50; i++) {
            write(1, "(", 1);
            raise(3);
        }
    }
}

int main(int argc, char** argv)
{
    testsignals();
    mem();
    exectest();
    fourfiles();
    dirfile();
    return 0;
}