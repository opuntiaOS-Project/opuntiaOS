#include "GraphView.h"
#include <algorithm>
#include <iostream>
#include <libfoundation/EventLoop.h>
#include <libfoundation/KeyboardMapping.h>
#include <libg/Color.h>
#include <libui/Context.h>

GraphView::GraphView(UI::View* superview, const LG::Rect& frame, int data_size)
    : UI::View(superview, frame)
    , m_data(data_size)
{
    for (int i = 0; i < data_size; i++) {
        m_data.push_back(0);
    }
}

void GraphView::display(const LG::Rect& rect)
{
    UI::Context ctx(*this);
    ctx.add_clip(rect);

    ctx.set_fill_color(LG::Color(233, 233, 233));
    ctx.fill(bounds());

    ctx.set_fill_color(LG::Color::LightSystemBlue);

    size_t left_padding = bounds().width();
    size_t height = bounds().height();
    size_t column_width = 3;

    for (int i = m_data.size() - 1; i >= 0; i--) {
        left_padding -= column_width;
        size_t column_height = (m_data[i] * height) / 100;
        ctx.fill(LG::Rect(left_padding, height - column_height, column_width, column_height));
        if (left_padding < column_width) {
            break;
        }
    }
}
