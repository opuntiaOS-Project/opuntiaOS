/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Context.h>

namespace WinServer {

class MenuWidget {
public:
    MenuWidget() = default;
    virtual ~MenuWidget() = default;

    virtual size_t width() { return 0; }
    virtual void draw(LG::Context& ctx) { }
};

} // namespace WinServer