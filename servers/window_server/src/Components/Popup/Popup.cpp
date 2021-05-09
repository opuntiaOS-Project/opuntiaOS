/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Popup.h"
#include "../Helpers/TextDrawer.h"
#include <algorithm>

namespace WinServer {

static Popup* s_the;

Popup& Popup::the()
{
    return *s_the;
}

Popup::Popup()
{
    s_the = this;
}

void Popup::draw(LG::Context& ctx)
{
    if (!visible()) {
        return;
    }

    ctx.set_fill_color(LG::Color(222, 222, 222, 180));
    ctx.mix(bounds());
}

} // namespace WinServer