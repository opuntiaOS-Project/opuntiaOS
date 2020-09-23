#include "Connection.h"
#include "Screen.h"
#include <malloc.h>
#include <syscalls.h>
#include <pthread.h>

void event_loop() {
    Connection::the().listen();
}

int main()
{
    new Screen();
    new Connection(socket(PF_LOCAL, 0, 0));
    pthread_create((void*)event_loop);    
    Screen::the().run();
    return 0;
}