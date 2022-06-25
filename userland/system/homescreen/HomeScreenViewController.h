/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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
#include <libfoundation/json/Parser.h>

class HomeScreenViewController : public UI::ViewController<HomeScreenView> {
public:
    HomeScreenViewController(HomeScreenView& view)
        : UI::ViewController<HomeScreenView>(view)
    {
        m_apps.push_back({
            .name = "about",
            .fast_launch = true,
            .home_screen = true,
        });

        m_apps.push_back({
            .name = "terminal",
            .fast_launch = true,
            .home_screen = true,
        });

        m_apps.push_back({
            .name = "activity_monitor",
            .fast_launch = false,
            .home_screen = true,
        });

        m_apps.push_back({
            .name = "calculator",
            .fast_launch = true,
            .home_screen = true,
        });
    }
    virtual ~HomeScreenViewController() = default;

    void init_data()
    {
        for (auto& app : m_apps) {
            std::string app_content_dir = "/Applications/";
            app_content_dir += app.name;
            app_content_dir += ".app/Content/";
            auto json_parser = LFoundation::Json::Parser(app_content_dir + "info.json");
            LFoundation::Json::Object* jobj_root = json_parser.object();

            if (jobj_root->invalid()) {
                return;
            }

            auto* jdict_root = jobj_root->cast_to<LFoundation::Json::DictObject>();

            std::string icon_path = jdict_root->data()["icon_path"]->cast_to<LFoundation::Json::StringObject>()->data();
            std::string icon_rel_path = jdict_root->data()["icon_rel_path"]->cast_to<LFoundation::Json::StringObject>()->data();

            if (!icon_rel_path.empty()) {
                icon_path = app_content_dir + icon_rel_path;
            }
            
            std::string exec_rel_path = jdict_root->data()["exec_rel_path"]->cast_to<LFoundation::Json::StringObject>()->data();
            std::string name = jdict_root->data()["name"]->cast_to<LFoundation::Json::StringObject>()->data();
            
            if (app.fast_launch) {
                view().new_fast_launch_entity(name, icon_path, app_content_dir + exec_rel_path);
            }

            if (app.home_screen) {
                view().new_grid_entity(name, icon_path, app_content_dir + exec_rel_path);
            }
        }
    }

    virtual void view_did_load() override
    {
        view().set_background_color(LG::Color(255, 255, 255, 135));
        init_data();
        view().set_needs_display();
    }

private:
    struct app_launcher {
        std::string name;
        bool fast_launch;
        bool home_screen;
    };
    typedef struct app_launcher app_launcher_t;
    std::vector<app_launcher_t> m_apps;
};