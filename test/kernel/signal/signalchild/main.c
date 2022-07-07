#include <sched.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

const int calls = 10;
volatile int ret = calls;

void save_on_terminate()
{
    ret--;
}

int main(int argc, char** argv)
{
    int pid = fork();
    if (pid) {
        for (int i = 0; i < 4; i++) {
            // Waiting until child setup its sigaction.
            sched_yield();
        }

        for (int i = 0; i < calls; i++) {
            kill(pid, 3);
            sched_yield();
        }

        waitpid(pid, (int*)&ret, 0);
    } else {
        sigaction(3, save_on_terminate);
        while (ret) { }
    }
    return ret;
}