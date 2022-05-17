#pragma once
#include "AppEntity.h"
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
    void mouse_up() override { on_click(); }

    static constexpr size_t underline_height() { return 4; }

    void set_title(const std::string& title)
    {
        if (!m_label) {
            return;
        }
        m_label->set_text(title);
    }

    AppEntity& entity() { return m_launch_entity; }
    const AppEntity& entity() const { return m_launch_entity; }

    virtual void mouse_entered(const LG::Point<int>& location) override
    {
        View::mouse_entered(location);
        set_needs_display();
    }

    virtual void mouse_exited() override
    {
        View::mouse_exited();
        set_needs_display();
    }

private:
    void on_click();
    void launch()
    {
        if (fork() == 0) {
            for (int i = 3; i < 32; i++) {
                close(i);
            }
            execlp(m_launch_entity.path_to_exec().c_str(), m_launch_entity.path_to_exec().c_str(), NULL);
            std::abort();
        }
    }

    UI::Label* m_label;
    AppEntity m_launch_entity;
};