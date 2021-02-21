/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Compositor.h"
#include "Connection.h"
#include "CursorManager.h"
#include "Devices.h"
#include "ResourceManager.h"
#include "Screen.h"
#include "WindowManager.h"
#include <libfoundation/EventLoop.h>
#include <malloc.h>
#include <pthread.h>
#include <syscalls.h>

void start_dock()
{
    if (fork()) {
        execve("/bin/dock", nullptr, nullptr);
        ASSERT_NOT_REACHED();
    }
}

int main()
{
    start_dock();
    auto* event_loop = new LFoundation::EventLoop();
    new Screen();
    new Connection(socket(PF_LOCAL, 0, 0));
    new CursorManager();
    new ResourceManager();
    new Compositor();
    new WindowManager();
    new Devices();
    event_loop->run();
    return 0;
}