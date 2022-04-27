#pragma once

#include "Exec.h"
#include <iostream>
#include <libfoundation/EventReceiver.h>
#include <vector>

namespace LaunchServer {

class LaunchWatchdog : public LFoundation::EventReceiver {
public:
    inline static LaunchWatchdog& the()
    {
        extern LaunchWatchdog* s_LaunchServer_LaunchWatchdog_the;
        return *s_LaunchServer_LaunchWatchdog_the;
    }

    LaunchWatchdog();
    ~LaunchWatchdog() = default;

    void tick()
    {
        for (auto& exec : m_execs) {
            if (!exec.is_alive()) {
                exec.launch();
            }
        }
    }

    void add(const Exec& exec) { m_execs.push_back(exec); }
    void add(Exec&& exec) { m_execs.push_back(std::move(exec)); }

private:
    std::vector<Exec> m_execs;
};

}