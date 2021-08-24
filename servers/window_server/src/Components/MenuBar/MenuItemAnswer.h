/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

namespace WinServer {

// Common for widget and panel item.
enum MenuItemAnswer {
    Empty = 0x0,
    Bad = 0x1, // Bad mark
    InvalidateMe = 0x2, // Asks to invalidate menu item
    PopupShow = 0x4, // Asks to show popup. MenuBar will call popup_rect()
    PopupClose = 0x8, // Asks to close popup.
};

}; // namespace WinServer