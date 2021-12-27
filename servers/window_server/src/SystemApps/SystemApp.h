/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "../Target/Generic/Window.h"

namespace WinServer {

class SystemApp {
public:
    SystemApp() = default;
    ~SystemApp() = default;

    void set_window(Window* win) { m_window = win; }
    Window* window() { return m_window; }
    const Window* window() const { return m_window; }

    bool has_value() const { return !!m_window; }
    operator bool() const { return has_value(); }

private:
    bool visible;
    Window* m_window {};
};

}