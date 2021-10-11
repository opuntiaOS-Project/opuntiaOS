#pragma once
#include <libg/Font.h>
#include <libui/View.h>
#include <string>

enum WindowStatus {
    Normal,
    DoNewLine,
};

class TerminalView : public UI::View {
    UI_OBJECT();

public:
    TerminalView(UI::View* superview, const LG::Rect&, int ptmx);
    TerminalView(UI::View* superview, UI::Window* window, const LG::Rect&, int ptmx);

    const LG::Color& font_color() const { return m_font_color; }
    const LG::Color cursor_color() const { return LG::Color(80, 80, 80, 255); }
    const LG::Color& background_color() const { return m_background_color; }
    inline const LG::Font& font() const { return *m_font_ptr; }

    inline int glyph_width() const { return font().glyph_width('.'); }
    inline int glyph_height() const { return font().glyph_height(); }

    inline LG::Point<int> pos_on_screen() const { return { (int)m_col * glyph_width() + padding(), (int)m_row * glyph_height() + padding() }; }
    inline int pos_in_data() const { return m_max_cols * m_row + m_col; }

    void put_char(char c);
    void put_text(const std::string& data);

    void display(const LG::Rect& rect) override;
    void receive_keyup_event(UI::KeyUpEvent&) override;
    void receive_keydown_event(UI::KeyDownEvent&) override;

    int ptmx() const { return m_ptmx; }

private:
    void terminal_init();

    WindowStatus cursor_positions_do_new_line();
    WindowStatus cursor_position_move_right();
    WindowStatus cursor_position_move_left();
    void data_do_new_line();
    inline void data_set_char(char c)
    {
        m_display_data[pos_in_data()] = c;
    }

    void scroll_line();
    void new_line();
    void increment_counter();
    void decrement_counter();

    void recalc_dimensions(const LG::Rect&);
    void push_back_char(char c);
    void send_input();

    inline void will_move_cursor()
    {
        auto pt = pos_on_screen();
        set_needs_display(LG::Rect(pt.x(), pt.y(), cursor_width() + spacing(), glyph_height()));
    }

    inline void did_move_cursor()
    {
        auto pt = pos_on_screen();
        set_needs_display(LG::Rect(pt.x(), pt.y(), cursor_width() + spacing(), glyph_height()));
    }

    LG::Color m_background_color { LG::Color(47, 47, 53) };
    LG::Color m_font_color { LG::Color::LightSystemText };
    LG::Font* m_font_ptr { LG::Font::load_from_file("/res/fonts/Liza.font/10/regular.font") };

    constexpr int padding() const { return 2; }
    constexpr int spacing() const { return 2; }
    constexpr int cursor_width() const { return 5; }

    int m_ptmx { -1 };
    std::string m_input {};

    size_t m_max_cols { 0 };
    size_t m_max_rows { 0 };
    size_t m_col { 0 };
    size_t m_row { 0 };
    char* m_display_data { nullptr };
};