#include "TerminalView.h"
#include <libfoundation/EventLoop.h>
#include <libfoundation/KeyboardMapping.h>
#include <libg/Color.h>
#include <libui/Context.h>
#include <std/Dbg.h>

static TerminalView* this_view;

TerminalView::TerminalView(const LG::Rect& frame, int ptmx)
    : View(frame)
    , m_ptmx(ptmx)
{
    this_view = this;
    LFoundation::EventLoop::the().add(
        ptmx, [] {
            char text[256];
            int cnt = read(this_view->ptmx(), text, 256);
            for (int i = 0; i < cnt; i++) {
                this_view->push_back_char(text[i]);
            }
        },
        nullptr);
    recalc_dimensions(frame);
}

void TerminalView::recalc_dimensions(const LG::Rect& frame)
{
    m_max_rows = frame.height() / glyph_height();
    m_max_cols = frame.width() / glyph_width();
    // FIXME: Add copy and resize on window resize.
    m_display_data = (char*)malloc(m_max_rows * m_max_cols);
    memset((uint8_t*)m_display_data, 0, m_max_rows * m_max_cols);
}

void TerminalView::display(const LG::Rect& rect)
{
    UI::Context ctx(*this);
    ctx.add_clip(rect);

    ctx.set_fill_color(background_color());
    ctx.fill(bounds());

    auto& f = font();
    ctx.set_fill_color(font_color());
    LG::Point<int> text_start { 0, 0 };

    for (int i = 0; i < m_max_rows; i++) {
        for (int j = 0; j < m_max_cols; j++) {
            int idx = i * m_max_cols + j;
            ctx.draw(text_start, f.glyph_bitmap(m_display_data[idx]));
            text_start.offset_by(glyph_width(), 0);
        }
        text_start.set_x(0);
        text_start.offset_by(0, glyph_height());
    }
}

void TerminalView::scroll_line()
{
    char* data_plus_line = m_display_data + (m_max_cols);
    char* data_end_minus_line = m_display_data + (m_max_rows - 1) * m_max_cols;
    memmove((uint8_t*)m_display_data, (uint8_t*)data_plus_line, (m_max_rows - 1) * m_max_cols);
    memset((uint8_t*)data_end_minus_line, 0, m_max_cols);
    set_needs_display(bounds());
}

void TerminalView::new_line()
{
    m_col = 0;
    m_row++;
    if (m_row == m_max_rows) {
        scroll_line();
        m_row--;
    }
}

void TerminalView::increment_counter()
{
    m_col++;
    if (m_col == m_max_cols) {
        new_line();
    }
}

void TerminalView::decrement_counter()
{
    m_col--;
    if (m_col == 0) {
        m_row--;
    }
}

void TerminalView::put_char(char c)
{
    auto pt = pos_on_screen();
    set_needs_display(LG::Rect(pt.x(), pt.y(), glyph_width(), glyph_height()));
    m_display_data[pos_in_data()] = c;
}

void TerminalView::push_back_char(char c)
{
    if (c == '\n') {
        new_line();
        return;
    }
    put_char(c);
    increment_counter();
}

void TerminalView::send_input()
{
    write(ptmx(), m_input.c_str(), m_input.size());
    m_input.clear();
}

void TerminalView::receive_keyup_event(UI::KeyUpEvent&)
{
}

void TerminalView::receive_keydown_event(UI::KeyDownEvent& event)
{
    // FIXME: More symbols and static size of font
    if (event.key() == LFoundation::Keycode::KEY_BACKSPACE) {
        if (m_input.size()) {
            m_input.pop_back();
            decrement_counter();
            put_char(' ');
        }
    } else if (event.key() == LFoundation::Keycode::KEY_RETURN) {
        push_back_char('\n');
        send_input();
    } else if (event.key() < 128) {
        m_input.push_back(char(event.key()));
        push_back_char(char(event.key()));
    }
}