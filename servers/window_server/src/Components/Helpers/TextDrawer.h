/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libg/Color.h>
#include <libg/Context.h>
#include <libg/Font.h>
#include <string>

namespace WinServer {
namespace Helpers {

    [[gnu::always_inline]] inline static size_t text_width(const std::string& text, const LG::Font& f)
    {
        size_t width = 0;
        for (int i = 0; i < text.size(); i++) {
            width += f.glyph_width(text[i]) + 2; // FIXME
        }
        return width;
    }

    [[gnu::always_inline]] inline static void draw_text(LG::Context& ctx, LG::Point<int> pt, const std::string& text, const LG::Font& f)
    {
        for (int i = 0; i < text.size(); i++) {
            ctx.draw(pt, f.glyph_bitmap(text[i]));
            pt.offset_by(f.glyph_width(text[i]) + 2, 0);
        }
    }

} // namespace Helpers
} // namespace WinServer