#pragma once
#include <libg/PixelBitmap.h>
#include <list>
#include <string>

class AppEntity {
public:
    AppEntity() = default;

    void set_icon(LG::PixelBitmap&& icon) { m_icon = std::move(icon); }
    const LG::PixelBitmap& icon() const { return m_icon; }

    void set_title(const std::string& title) { m_title = title; }
    const std::string& title() const { return m_title; }

    void set_path_to_exec(const std::string& path) { m_path_to_exec = path; }
    const std::string& path_to_exec() const { return m_path_to_exec; }

    void set_bundle_id(const std::string& bid) { m_bundle_id = bid; }
    const std::string& bundle_id() const { return m_bundle_id; }

private:
    LG::PixelBitmap m_icon;
    std::string m_title {};
    std::string m_path_to_exec {};
    std::string m_bundle_id {};
};