#pragma once
#include <libg/PixelBitmap.h>
#include <libg/string.h>

class FastLaunchEntity {
public:
    FastLaunchEntity() = default;

    void set_icon(LG::PixelBitmap&& icon) { m_icon = std::move(icon); }
    const LG::PixelBitmap& icon() const { return m_icon; }

    void set_path_to_exec(LG::string&& path) { m_path_to_exec = std::move(path); }
    const LG::string& path_to_exec() const { return m_path_to_exec; }

private:
    LG::PixelBitmap m_icon;
    LG::string m_path_to_exec {};
};