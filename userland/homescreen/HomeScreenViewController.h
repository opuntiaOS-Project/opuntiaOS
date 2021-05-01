/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "HomeScreenView.h"
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/ViewController.h>
#include <libui/Window.h>
#include <memory>
#include <sys/types.h>

class HomeScreenViewController : public UI::ViewController<HomeScreenView> {
public:
    HomeScreenViewController(HomeScreenView& view)
        : UI::ViewController<HomeScreenView>(view)
    {
    }
    virtual ~HomeScreenViewController() = default;

    virtual void view_did_load() override
    {
        view().set_background_color(LG::Color(222, 222, 222, 180));
        view().new_fast_launch_entity("/res/icons/apps/about.icon", "/bin/about"); // FIXME: Parse some init file
        view().new_fast_launch_entity("/res/icons/apps/terminal.icon", "/bin/terminal");
        view().new_fast_launch_entity("/res/icons/apps/activity_monitor.icon", "/bin/activity_monitor");
        view().set_needs_display();
    }

private:
};