/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../shared/Connections/WSConnection.h"
#include "ServerDecoder.h"
#include <libipc/ServerConnection.h>
#include <vector>

namespace WinServer {

class CursorManager;
class ResourceManager;
class MenuBar;
#ifdef TARGET_MOBILE
class ControlBar;
#endif // TARGET_MOBILE
class Popup;

class Compositor {
public:
    static Compositor& the();
    Compositor();

    void refresh();

    void optimized_invalidate_insert(std::vector<LG::Rect>& data, const LG::Rect& inv_area)
    {
        auto area = inv_area;
        bool intersects = false;
        int int_index = 0;
        for (auto& rect : data) {
            if (rect.contains(area)) {
                return;
            }
        }

        for (auto& rect : data) {
            if (rect.intersects(area)) {
                intersects = true;
                break;
            }
            int_index++;
        }

        if (!intersects) {
            data.push_back(area);
            return;
        }

        data[int_index].unite(area);
        for (int i = int_index + 1; i < data.size(); i++) {
            if (data[int_index].intersects(data[i])) {
                data[int_index].unite(data[i]);
                std::swap(data[i], data.back());
                data.pop_back();
                i--;
            }
        }
    }

    inline void invalidate(const LG::Rect& area) { optimized_invalidate_insert(m_invalidated_areas, area); }
    inline CursorManager& cursor_manager() { return m_cursor_manager; }
    inline const CursorManager& cursor_manager() const { return m_cursor_manager; }
    inline ResourceManager& resource_manager() { return m_resource_manager; }
    inline const ResourceManager& resource_manager() const { return m_resource_manager; }
    inline Popup& popup() { return m_popup; }
    inline const Popup& popup() const { return m_popup; }
    inline MenuBar& menu_bar() { return m_menu_bar; }
    inline const MenuBar& menu_bar() const { return m_menu_bar; }
#ifdef TARGET_MOBILE
    inline ControlBar& control_bar()
    {
        return m_control_bar;
    }
    inline const ControlBar& control_bar() const { return m_control_bar; }
#endif // TARGET_MOBILE

private:
    void copy_changes_to_second_buffer(const std::vector<LG::Rect>& areas);

    std::vector<LG::Rect> m_invalidated_areas;
    MenuBar& m_menu_bar;
    Popup& m_popup;
    CursorManager& m_cursor_manager;
    ResourceManager& m_resource_manager;

#ifdef TARGET_MOBILE
    ControlBar& m_control_bar;
#endif // TARGET_MOBILE
};

} // namespace WinServer