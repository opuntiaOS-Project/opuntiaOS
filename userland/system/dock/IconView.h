#pragma once
#include "DockEntity.h"
#include "WindowEntity.h"
#include <libg/Font.h>
#include <libui/Label.h>
#include <libui/PopupMenu.h>
#include <libui/View.h>
#include <list>
#include <string>
#include <unistd.h>

class IconView : public UI::View {
    UI_OBJECT();

public:
    IconView(View* superview, const LG::Rect& frame);

    void display(const LG::Rect& rect) override;
    void mouse_down(const LG::Point<int>& location) override { on_click(); }

    static constexpr size_t underline_height() { return 3; }

    void set_title(const std::string& title)
    {
        if (!m_label) {
            return;
        }
        m_label->set_text(title);
    }

    DockEntity& entity() { return m_launch_entity; }
    const DockEntity& entity() const { return m_launch_entity; }

private:
    void on_click();
    void launch()
    {
        if (fork() == 0) {
            execve(m_launch_entity.path_to_exec().c_str(), 0, 0);
            std::abort();
        }
    }

    UI::Label* m_label;
    DockEntity m_launch_entity;
};