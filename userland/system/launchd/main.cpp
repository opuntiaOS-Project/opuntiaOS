#include "LaunchWatchdog.h"
#include <libfoundation/EventLoop.h>
#include <libfoundation/json/Parser.h>
#include <new>

void load_from_file(LaunchServer::LaunchWatchdog& launch_watchdog)
{
    auto json_parser = LFoundation::Json::Parser("/System/launchd_config.json");
    LFoundation::Json::Object* jobj_root = json_parser.object();
    if (jobj_root->invalid()) {
        std::abort();
    }

    auto* jdict_root = jobj_root->cast_to<LFoundation::Json::DictObject>();
    auto* jlaunch_list = jdict_root->data()["launch"]->cast_to<LFoundation::Json::ListObject>();
    for (auto* jobj : jlaunch_list->data()) {
        const std::string& strdata = jobj->cast_to<LFoundation::Json::StringObject>()->data();
        launch_watchdog.add(LaunchServer::Exec(strdata, LaunchServer::Exec::Flags::RestartOnFail));
    }
}

int main(int argc, char** argv)
{
    auto* event_loop = new LFoundation::EventLoop();
    auto* launch_watchdog = new LaunchServer::LaunchWatchdog();

    load_from_file(*launch_watchdog);
    launch_watchdog->tick();
    event_loop->add(LFoundation::Timer([launch_watchdog] {
        launch_watchdog->tick();
    },
        5000, LFoundation::Timer::Repeat));
    return event_loop->run();
}