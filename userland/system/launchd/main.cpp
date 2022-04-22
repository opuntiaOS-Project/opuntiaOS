#include "LaunchWatchdog.h"
#include <libfoundation/EventLoop.h>
#include <new>

int main(int argc, char** argv)
{
    auto* event_loop = new LFoundation::EventLoop();
    auto* launch_watchdog = new LaunchServer::LaunchWatchdog();

    // TODO: Read from file.
    launch_watchdog->add(LaunchServer::Exec("/System/window_server", LaunchServer::Exec::Flags::RestartOnFail));
    launch_watchdog->tick();
    event_loop->add(LFoundation::Timer([launch_watchdog] {
        launch_watchdog->tick();
    },
        5000, LFoundation::Timer::Repeat));
    return event_loop->run();
}