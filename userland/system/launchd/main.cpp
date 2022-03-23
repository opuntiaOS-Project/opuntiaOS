#include <new>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

void on_main_start();

enum LAUNCHENTRY_FLAGS {
    EMPTY = 0x0,
    RESTART_ON_FAILURE = 0x1,
};

struct launchentry {
    const char* path;
    uint32_t flags;
};
typedef struct launchentry launchentry_t;

struct runentry {
    const launchentry_t* launchentry;
    pid_t pid;
    struct runentry* next;
};
typedef struct runentry runentry_t;

static runentry_t* head = nullptr;
static runentry_t* last = nullptr;

launchentry_t lentries[] = {
    { "/System/window_server", RESTART_ON_FAILURE },
};
#define LAUNCH_ENTRIES_SIZE (sizeof(lentries) / sizeof(launchentry_t))

int runentry_add(const launchentry_t* launchentry, pid_t pid)
{
    runentry_t* renew = new runentry_t();
    renew->launchentry = launchentry;
    renew->pid = pid;
    renew->next = nullptr;

    if (last == NULL) {
        head = last = renew;
        return 0;
    }

    last->next = renew;
    last = renew;
    return 0;
}

int launch(const launchentry_t* launchentry)
{
    int pid = fork();
    if (pid == 0) {
        execlp(launchentry->path, launchentry->path, NULL);
        exit(1);
    }

    runentry_add(launchentry, pid);
    return 0;
}

int main(int argc, char** argv)
{
    for (int i = 0; i < LAUNCH_ENTRIES_SIZE; i++) {
        launch(&lentries[i]);
    }

    for (;;) {
        runentry_t* s = head;
        while (s) {
            if (kill(s->pid, 0) != 0) {
                // TODO: Remove from the list.
                launch(s->launchentry);
            }
            s = s->next;
        }

        sleep(1);
    }

    return 0;
}