/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <string>
#include <unistd.h>
#include <vector>

namespace LFoundation {

class ProcessInfo {
public:
    inline static ProcessInfo& the()
    {
        extern ProcessInfo* s_LFoundation_ProcessInfo_the;
        return *s_LFoundation_ProcessInfo_the;
    }

    ProcessInfo(int argc, char** argv);
    ~ProcessInfo() = default;

    std::vector<std::string>& arguments() { return m_args; }
    const std::vector<std::string>& arguments() const { return m_args; }

    std::string& process_name() { return m_process_name; }
    const std::string& process_name() const { return m_process_name; }

    int processor_count()
    {
        // TODO: Temp solution. Until sysctl.
        if (m_processor_count < 0) {
            m_processor_count = 0;
            char buf[256];
            int fd_proc_stat = open("/proc/stat", O_RDONLY);
            int offset = 0;
            int rd = 1;
            read(fd_proc_stat, buf, sizeof(buf));
            while (rd > 0) {
                int num, user_time, system_time, idle_time;
                rd = sscanf(buf + offset, "cpu%d %d 0 %d %d\n", &num, &user_time, &system_time, &idle_time);
                offset += rd;
                if (rd > 0) {
                    m_processor_count++;
                }
            }
        }
        return m_processor_count;
    }

    bool mobile_app_on_desktop() { return false; }

private:
    std::vector<std::string> m_args;
    std::string m_process_name;
    int m_processor_count { -1 };
};

} // namespace LFoundation