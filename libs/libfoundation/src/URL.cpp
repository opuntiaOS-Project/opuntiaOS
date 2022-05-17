/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/URL.h>
#include <string>

namespace LFoundation {

URL::Scheme URL::parse_scheme(const std::string& path)
{
    if (path.starts_with("file://")) {
        return Scheme::File;
    } else if (path.starts_with("http://")) {
        return Scheme::Http;
    } else if (path.starts_with("https://")) {
        return Scheme::Https;
    }

    // Unknown scheme.
    std::abort();
}

} // namespace LFoundation