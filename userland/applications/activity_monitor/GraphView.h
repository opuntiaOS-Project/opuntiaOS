#pragma once
#include <libg/Font.h>
#include <libui/View.h>
#include <string>

class GraphView : public UI::View {
    UI_OBJECT();

public:
    GraphView(UI::View* superview, const LG::Rect&, int data_size);

    void display(const LG::Rect& rect) override;

    void add_new_value(int val)
    {
        for (int i = 0; i < m_data.size() - 1; i++) {
            m_data[i] = m_data[i + 1];
        }
        m_data[m_data.size() - 1] = val;
        set_needs_display();
    }

private:
    std::vector<int> m_data;
};