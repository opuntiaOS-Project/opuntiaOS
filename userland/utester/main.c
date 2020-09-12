#include <libsystem/syscalls.h>
#include <libsystem/malloc.h>

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

int main(int argc, char** argv)
{
    mem();
    exitwait();
    return 0;
}