/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Components/ControlBar/ControlBar.h"
#include "Components/LoadingScreen/LoadingScreen.h"
#include "Components/MenuBar/MenuBar.h"
#include "Components/MenuBar/Widgets/Clock/Clock.h"
#include "Components/Popup/Popup.h"
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

#ifdef TARGET_DESKTOP
#define LAUNCH_PATH "/System/dock"
#elif TARGET_MOBILE
#define LAUNCH_PATH "/System/homescreen"
#endif

void start_dock()
{
    if (fork()) {
        execve(LAUNCH_PATH, nullptr, nullptr);
        std::abort();
    }
}

void screen_init()
{
    start_dock();
    nice(-3);
    new WinServer::Screen();
    new WinServer::LoadingScreen();
}

template <class T, int Cost = 1, class... Args>
constexpr inline void load_core_component(Args&&... args)
{
    new T(std::forward<Args>(args)...);
    WinServer::LoadingScreen::the().move_progress<T, Cost>();
}

template <class T, int Cost = 1, class... Args>
constexpr inline void add_widget(Args&&... args)
{
    WinServer::MenuBar::the().add_widget<T>(std::forward<Args>(args)...);
    WinServer::LoadingScreen::the().move_progress<T, Cost>();
}

int main()
{
    screen_init();
    auto* event_loop = new LFoundation::EventLoop();
    load_core_component<WinServer::Connection>(socket(PF_LOCAL, 0, 0));
    load_core_component<WinServer::CursorManager>();
    load_core_component<WinServer::ResourceManager, 4>();
    load_core_component<WinServer::Popup>();
    load_core_component<WinServer::MenuBar>();
#ifdef TARGET_MOBILE
    load_core_component<WinServer::ControlBar>();
#endif
    load_core_component<WinServer::Compositor>();
    load_core_component<WinServer::WindowManager>();
    load_core_component<WinServer::Devices>();

    add_widget<WinServer::Clock>();

    WinServer::LoadingScreen::destroy_the();
    return event_loop->run();
}