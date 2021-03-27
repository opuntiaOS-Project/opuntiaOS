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
#include <cstdlib>
#include <libfoundation/EventLoop.h>
#include <new>
#include <sys/socket.h>
#include <unistd.h>

void start_dock()
{
    if (fork()) {
        execve("/bin/dock", nullptr, nullptr);
        std::abort();
    }
}

int main()
{
    start_dock();
    auto* event_loop = new LFoundation::EventLoop();
    new WinServer::Screen();
    new WinServer::Connection(socket(PF_LOCAL, 0, 0));
    new WinServer::CursorManager();
    new WinServer::ResourceManager();
    new WinServer::Compositor();
    new WinServer::WindowManager();
    new WinServer::Devices();
    event_loop->run();
    return 0;
}