/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../Components/Base/BaseWindow.h"
#include "../Components/MenuBar/MenuItem.h"
#include "../Connection.h"
#include <libfoundation/SharedBuffer.h>
#include <libg/PixelBitmap.h>
#include <libg/Rect.h>
#include <std/WeakPtr.h>
#include <sys/types.h>
#include <utility>

namespace WinServer::Mobile {

class Window : public BaseWindow, public Weakable<Window> {
public:
    Window(int connection_id, int id, const CreateWindowMessage& msg);
    Window(Window&& win);

    inline void offset_by(int x_offset, int y_offset)
    {
        bounds().offset_by(x_offset, y_offset);
        content_bounds().offset_by(x_offset, y_offset);
    }

private:
};

} // namespace WinServer