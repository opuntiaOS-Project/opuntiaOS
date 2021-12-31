/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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

    const std::string& process_name() const { return m_process_name; }
    const std::string& bundle_id() const { return m_bundle_id; }

    int processor_count();

    bool mobile_app_on_desktop() { return false; }

private:
    // TODO: Maybe move out info file parsing to a seperate file?
    void parse_info_file();

    std::vector<std::string> m_args;
    std::string m_process_name;
    std::string m_bundle_id;
    int m_processor_count { -1 };
};

} // namespace LFoundation