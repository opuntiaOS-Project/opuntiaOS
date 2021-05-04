/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "GraphView.h"
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/ViewController.h>
#include <libui/Window.h>
#include <memory>
#include <sys/types.h>

static char buf[256];

class ViewController : public UI::ViewController<UI::View> {
public:
    ViewController(UI::View& view)
        : UI::ViewController<UI::View>(view)
    {
    }
    virtual ~ViewController() = default;

    void view_did_load() override
    {
        view().set_background_color(LG::Color::LightSystemBackground);

        auto& cpu_label = view().add_subview<UI::Label>(LG::Rect(0, 0, 180, 16));
        auto& cpu_graph = view().add_subview<GraphView>(LG::Rect(0, 0, 184, 100), 92);

        view().add_constraint(UI::Constraint(cpu_label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
        view().add_constraint(UI::Constraint(cpu_label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, UI::SafeArea::Top));

        view().add_constraint(UI::Constraint(cpu_graph, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
        view().add_constraint(UI::Constraint(cpu_graph, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, cpu_label, UI::Constraint::Attribute::Bottom, 1, 8));

        view().set_needs_layout();

        UI::App::the().event_loop().add(LFoundation::Timer([&] {
            fetch_data();
            cpu_label.set_text(std::string("Cpu load ") + std::to_string(state.cpu_load) + "%");
            cpu_label.set_needs_display();
            cpu_graph.add_new_value(state.cpu_load);
            cpu_graph.set_needs_display();
        },
            1000, LFoundation::Timer::Repeat));
    }

    int get_cpu_load()
    {
        int fd_proc_stat = open("/proc/stat", O_RDONLY);
        read(fd_proc_stat, buf, sizeof(buf));

        int user_time, system_time, idle_time;
        sscanf(buf, "cpu0 %d 0 %d %d", &user_time, &system_time, &idle_time);

        int diff_user_time = user_time - old_user_time;
        int diff_system_time = system_time - old_system_time;
        int diff_idle_time = idle_time - old_idle_time;
        old_user_time = user_time;
        old_system_time = system_time;
        old_idle_time = idle_time;

        if (diff_user_time + diff_system_time + diff_idle_time == 0) {
            state.cpu_load = 0;
        } else {
            state.cpu_load = (diff_user_time + diff_system_time) * 100 / (diff_user_time + diff_system_time + diff_idle_time);
        }

        close(fd_proc_stat);
        return 0;
    }

    void fetch_data()
    {
        get_cpu_load();
    }

private:
    int fd_proc_stat;
    int old_user_time = 0, old_system_time = 0, old_idle_time = 0;

    struct State {
        int cpu_load;
    };
    State state;
};