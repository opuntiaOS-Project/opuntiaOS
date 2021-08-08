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
#include <libui/StackView.h>
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

    void init_data()
    {
        view().new_fast_launch_entity("About", "/res/icons/apps/about.icon", "/Applications/about.app/Content/about"); // FIXME: Parse some init file
        view().new_fast_launch_entity("Terminal", "/res/icons/apps/terminal.icon", "/Applications/terminal.app/Content/terminal");
        view().new_fast_launch_entity("Calculator", "/res/icons/apps/calculator.icon", "/Applications/calculator.app/Content/calculator");
        view().new_grid_entity("About", "/res/icons/apps/about.icon", "/Applications/about.app/Content/about"); // FIXME: Parse some init file
        view().new_grid_entity("Terminal", "/res/icons/apps/terminal.icon", "/Applications/terminal.app/Content/terminal");
        view().new_grid_entity("Monitor", "/res/icons/apps/activity_monitor.icon", "/Applications/activity_monitor.app/Content/activity_monitor");
        view().new_grid_entity("Calculator", "/res/icons/apps/calculator.icon", "/Applications/calculator.app/Content/calculator");
    }

    virtual void view_did_load() override
    {
        view().set_background_color(LG::Color(255, 255, 255, 135));
        init_data();
        view().set_needs_display();
    }

private:
};