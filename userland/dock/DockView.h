#pragma once
#include "DockEntity.h"
#include <libg/Font.h>
#include <libui/View.h>
#include <std/LinkedList.h>
#include <std/String.h>

class DockView : public UI::View {
public:
    DockView(const LG::Rect&);

    void display(const LG::Rect& rect) override;
    
    DockEntity* find_entity(int window_id);
    void new_entity(int window_id);
    void remove_entity(int window_id);
    void set_icon(int window_id, const LG::String& path);

private:
    LG::Color m_font_color { LG::Color::White };
    LG::Font* m_font_ptr { LG::Font::load_from_file("/res/LizaRegular8x10.font") };
    LinkedList<DockEntity> m_dock_entites {};
};