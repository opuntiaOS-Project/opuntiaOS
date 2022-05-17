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

class URL {
public:
    enum Scheme {
        Http,
        Https,
        File,
    };

    explicit URL(const std::string& url)
        : m_url(url)
    {
        m_scheme = parse_scheme(url);
    }
    ~URL() = default;

    bool is_file() { return m_scheme == Scheme::File; }

    const std::string& url() const { return m_url; }
    const Scheme& scheme() const { return m_scheme; }

private:
    Scheme parse_scheme(const std::string& path);

    std::string m_url;
    Scheme m_scheme;
};

} // namespace LFoundation