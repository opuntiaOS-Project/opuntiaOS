/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Compositor.h"
#include "Connection.h"
#include "Devices.h"
#include "Screen.h"
#include "WindowManager.h"
#include <libfoundation/EventLoop.h>
#include <malloc.h>
#include <pthread.h>
#include <syscalls.h>

void event_loop()
{
    Connection::the().listen();
}

int main()
{
    auto event_loop = new LFoundation::EventLoop();
    new Screen();
    new Connection(socket(PF_LOCAL, 0, 0));
    new WindowManager();
    new Compositor();
    new Devices();
    event_loop->run();
    return 0;
}