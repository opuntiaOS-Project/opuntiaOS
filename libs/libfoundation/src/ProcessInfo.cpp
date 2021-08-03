/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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

static ProcessInfo* s_the = nullptr;

ProcessInfo& ProcessInfo::the()
{
    return *s_the;
}

ProcessInfo::ProcessInfo(int argc, char** argv)
    : m_process_name(argv[0])
{
    s_the = this;
    for (int i = 1; i < argc; i++) {
        m_args.push_back(std::string(argv[i]));
    }
}

} // namespace LFoundation