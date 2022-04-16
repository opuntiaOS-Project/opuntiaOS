/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libui/GestureRecognizer.h>

namespace UI {

class GestureManager final {
public:
    GestureManager() = default;
    ~GestureManager()
    {
        for (auto* gst : m_handlers) {
            delete gst;
        }
    }

    inline void add(GestureRecognizer* recon) { m_handlers.push_back(recon); }
    void mouse_up()
    {
        for (auto* handler : m_handlers) {
            handler->mouse_up();
        }
    }

    void mouse_down(const LG::Point<int>& location)
    {
        for (auto* handler : m_handlers) {
            handler->mouse_down(location);
        }
    }

    void mouse_moved(const LG::Point<int>& new_location)
    {
        for (auto* handler : m_handlers) {
            handler->mouse_moved(new_location);
        }
    }

private:
    std::vector<GestureRecognizer*> m_handlers;
};

} // namespace UI