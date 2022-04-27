#pragma once

#include <csignal>
#include <string>
#include <unistd.h>
#include <vector>

namespace LaunchServer {

class Exec {
public:
    enum Flags {
        None = 0,
        RestartOnFail = (1 << 0),
    };

    Exec(const std::string& path, Flags flag = Flags::None)
        : m_path(path)
        , m_flags(flag)
    {
    }
    ~Exec() = default;

    void launch()
    {
        m_pid = fork();
        if (m_pid == 0) {
            execlp(m_path.c_str(), m_path.c_str(), NULL);
            std::abort();
        }
    }

    bool is_alive() const { return m_pid > 0 && kill(m_pid, 0) == 0; }

private:
    std::string m_path;
    Flags m_flags;
    int m_pid { -1 };
};

}