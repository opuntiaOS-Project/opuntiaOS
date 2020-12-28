#pragma once
#include <libg/PixelBitmap.h>
#include <std/LinkedList.h>

class DockEntity : public LinkedListNode<DockEntity> {
public:
    DockEntity(int window_id)
        : m_window_id(window_id)
    {
    }

    int window_id() const { return m_window_id; }
    void set_icon(LG::PixelBitmap&& icon) { m_icon = move(icon); }
    const LG::PixelBitmap& icon() const { return m_icon; }

private:
    int m_window_id;
    LG::PixelBitmap m_icon;
    int m_window_status;
};