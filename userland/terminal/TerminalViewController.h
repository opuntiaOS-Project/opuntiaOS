/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "TerminalView.h"
#include <libui/ViewController.h>
#include <memory>
#include <sys/types.h>

class TerminalViewController : public UI::ViewController<TerminalView> {
public:
    TerminalViewController(TerminalView& view)
        : UI::ViewController<TerminalView>(view)
    {
    }
    virtual ~TerminalViewController() = default;

    void view_did_load() override
    {
    }

private:
};