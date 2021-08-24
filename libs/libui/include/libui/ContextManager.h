/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Context.h>

namespace UI {

static inline void graphics_push_context(LG::Context&& context)
{
    extern std::vector<LG::Context> s_ui_graphics_contexts;
    s_ui_graphics_contexts.push_back(std::move(context));
}

static inline void graphics_push_context(const LG::Context& context)
{
    extern std::vector<LG::Context> s_ui_graphics_contexts;
    s_ui_graphics_contexts.push_back(context);
}

static inline void graphics_pop_context()
{
    extern std::vector<LG::Context> s_ui_graphics_contexts;
    s_ui_graphics_contexts.pop_back();
}

static inline LG::Context& graphics_current_context()
{
    extern std::vector<LG::Context> s_ui_graphics_contexts;
    return s_ui_graphics_contexts.back();
}

} // namespace UI