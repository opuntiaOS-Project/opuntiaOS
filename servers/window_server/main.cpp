/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Compositor.h"
#include "Connection.h"
#include "Screen.h"
#include "WindowManager.h"
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
    new WindowManager();
    new Compositor();
    new Connection(socket(PF_LOCAL, 0, 0));
    pthread_create((void*)event_loop);
    for (;;) {
        Compositor::the().refresh();
    }
    return 0;
}