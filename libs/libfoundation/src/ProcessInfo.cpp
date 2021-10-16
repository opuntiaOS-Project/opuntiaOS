/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstring>
#include <iostream>
#include <libfoundation/Logger.h>
#include <libfoundation/ProcessInfo.h>
#include <memory>
#include <unistd.h>

namespace LFoundation {

ProcessInfo* s_LFoundation_ProcessInfo_the = nullptr;

ProcessInfo::ProcessInfo(int argc, char** argv)
{
    s_LFoundation_ProcessInfo_the = this;

    // Parse argv[0] to get a process name.
    int process_name_start = 0;
    for (int i = 0; i < strlen(argv[0]) - 1; i++) {
        if (argv[0][i] == '/') {
            process_name_start = i + 1;
        }
    }
    m_process_name = std::string(&argv[0][process_name_start]);

    for (int i = 1; i < argc; i++) {
        m_args.push_back(std::string(argv[i]));
    }

    parse_info_file();
}

int ProcessInfo::processor_count()
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
        close(fd_proc_stat);
    }
    return m_processor_count;
}

void ProcessInfo::parse_info_file()
{
    char execpath[256];
    char infofile[256];
    char bundleid[256];

    int fd = open("/proc/self/exe", O_RDONLY);
    int rd = read(fd, execpath, sizeof(execpath));
    close(fd);
    int start = 0;
    for (int i = rd - 1; i >= 0; i--) {
        if (execpath[i] == '/') {
            start = i + 1;
            break;
        }
    }
    memcpy(&execpath[start], "info.json", sizeof("info.json") + 1);

    fd = open(execpath, O_RDONLY);
    if (fd <= 0) {
        return;
    }
    rd = read(fd, infofile, sizeof(infofile));
    close(fd);

    sscanf(infofile, "bundle_id: %s", bundleid);
    m_bundle_id = std::string(bundleid);
}

} // namespace LFoundation