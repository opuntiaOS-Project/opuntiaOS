/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "../Desktop/Window.h"
#include "../Mobile/Window.h"

namespace WinServer {

#ifdef TARGET_DESKTOP
using Window = Desktop::Window;
#elif TARGET_MOBILE
using Window = Mobile::Window;
#endif

} // namespace WinServer