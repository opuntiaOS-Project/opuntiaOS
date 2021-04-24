#include "GraphView.h"
#include <iostream>
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <memory>

#define BODY_COLOR 0x819EFA

static char buf[256];
int fd_proc_stat;
int old_user_time = 0, old_system_time = 0, old_idle_time = 0;

struct State {
    int cpu_load;
};
static State state;

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

int main(int argc, char** argv)
{
    auto& app = std::oneos::construct<UI::App>();
    auto& window = std::oneos::construct<UI::Window>(200, 140, "/res/icons/apps/activity_monitor.icon");
    auto& superview = window.create_superview<UI::View>();
    superview.set_background_color(LG::Color::LightSystemBackground);

    auto& cpu_label = superview.add_subview<UI::Label>(LG::Rect(0, 0, 180, 16));
    auto& cpu_graph = superview.add_subview<GraphView>(LG::Rect(0, 0, 184, 100), 92);

    superview.add_constraint(UI::Constraint(cpu_label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
    superview.add_constraint(UI::Constraint(cpu_label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, UI::SafeArea::Top));

    superview.add_constraint(UI::Constraint(cpu_graph, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
    superview.add_constraint(UI::Constraint(cpu_graph, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, cpu_label, UI::Constraint::Attribute::Bottom, 1, 8));

    window.superview()->set_needs_layout();
    window.set_title("Monitor");

    app.event_loop().add(LFoundation::Timer([&] {
        fetch_data();
        cpu_label.set_text(std::string("Cpu load ") + std::to_string(state.cpu_load) + "%");
        cpu_label.set_needs_display();
        cpu_graph.add_new_value(state.cpu_load);
        cpu_graph.set_needs_display();
    }, 1000, LFoundation::Timer::Repeat));

    return app.run();
}