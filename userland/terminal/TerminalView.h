#pragma once
#include <libg/Font.h>
#include <libui/View.h>

class TerminalView : public UI::View {
public:
    TerminalView(const LG::Rect&);

    const LG::Color& font_color() const { return m_font_color; }
    const LG::Color& background_color() const { return m_background_color; }
    inline const LG::Font& font() const { return *m_font_ptr; }

    inline int glyph_width() const { return font().glyph_width('.'); }
    inline int glyph_height() const { return font().glyph_height(); }

    inline LG::Point<int> pos_on_screen() const { return { m_col * glyph_width(), m_row * glyph_height() }; }
    inline int pos_in_data() const { return m_max_cols * m_row + m_col; }

    void display(const LG::Rect& rect) override;
    void receive_keyup_event(UI::KeyUpEvent&) override;
    void receive_keydown_event(UI::KeyDownEvent&) override;

private:
    void scroll_line();
    void new_line();
    void increment_counter();

    void recalc_dimensions(const LG::Rect&);
    void add_char(char c);

    LG::Color m_background_color { LG::Color::Black };
    LG::Color m_font_color { LG::Color::White };
    LG::Font* m_font_ptr { LG::Font::load_from_file("/res/LizaRegular8x10.font") };

    size_t m_max_cols { 0 };
    size_t m_max_rows { 0 };
    size_t m_col { 0 };
    size_t m_row { 0 };
    char* m_display_data { nullptr };
};