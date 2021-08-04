/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "../../Compositor.h"
#include <functional>
#include <libg/Color.h>
#include <libg/Context.h>
#include <string>
#include <vector>

namespace WinServer {

struct PopupItem {
    std::string text;
    std::function<void(int)> callback;
};
using PopupData = std::vector<PopupItem>;

class Popup {
public:
    inline static Popup& the()
    {
        extern Popup* s_WinServer_Popup_the;
        return *s_WinServer_Popup_the;
    }

    Popup();
    ~Popup() = default;

    inline void set_preferred_origin(const LG::Point<int>& origin) { m_bounds.set_origin(origin); }
    inline void set_preferred_origin(LG::Point<int>&& origin) { m_bounds.set_origin(origin); }
    inline LG::Rect& bounds() { return m_bounds; }
    inline const LG::Rect& bounds() const { return m_bounds; }

    inline void set_visible(bool vis)
    {
        if (m_visible != vis) {
            Compositor::the().invalidate(bounds());
        }
        m_visible = vis;
    }
    inline bool visible() const { return m_visible; }

    void draw(LG::Context& ctx);

private:
    LG::Rect m_bounds { 0, 0, 200, 170 };
    bool m_visible { false };
    PopupData m_data;
};

} // namespace WinServer