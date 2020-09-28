#include "Compositor.h"
#include "Connection.h"
#include "Screen.h"
#include <malloc.h>
#include <pthread.h>
#include <syscalls.h>

void event_loop()
{
    Connection::the().listen();
}

int main()
{
    new Screen();
    new Compositor();
    new Connection(socket(PF_LOCAL, 0, 0));
    pthread_create((void*)event_loop);
    for (;;) {
        Compositor::the().refresh();
    }
    return 0;
}