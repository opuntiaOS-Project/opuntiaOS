/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "WSConnection.h"
#include "WSServerDecoder.h"
#include <libipc/ServerConnection.h>
#include <std/Vector.h>
#include <syscalls.h>

class CursorManager;
class ResourceManager;

class Compositor {
public:
    static Compositor& the();
    Compositor();

    void refresh();

    void optimized_invalidate_insert(Vector<LG::Rect>& data, const LG::Rect& area)
    {
        LG::Rect area_union;
        size_t area_square = area.square();
        size_t inv_area_square = 0;

        for (int i = 0; i < data.size(); i++) {
            inv_area_square = data[i].square();
            area_union = area.union_of(data[i]);
            if (area_square + inv_area_square > area_union.square()) {
                data[i].unite(area);
                return;
            }
        }

        data.push_back(area);
    }

    inline void invalidate(const LG::Rect& area) { optimized_invalidate_insert(m_invalidated_areas, area); }
    inline CursorManager& cursor_manager() { return m_cursor_manager; }
    inline const CursorManager& cursor_manager() const { return m_cursor_manager; }
    inline ResourceManager& resource_manager() { return m_resource_manager; }
    inline const ResourceManager& resource_manager() const { return m_resource_manager; }

private:
    void copy_changes_to_second_buffer(const Vector<LG::Rect>& areas);

    CursorManager& m_cursor_manager;
    ResourceManager& m_resource_manager;
    Vector<LG::Rect> m_invalidated_areas;
};