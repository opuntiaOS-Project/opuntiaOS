#pragma once
#include "FastLaunchEntity.h"
#include "HomeScreenEntity.h"
#include <libg/Font.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <list>
#include <string>
#include <unistd.h>

class IconView : public UI::View {
    UI_OBJECT();

public:
    IconView(View* superview, const LG::Rect& frame);

    void display(const LG::Rect& rect) override;
    void click_began(const LG::Point<int>& location) override { launch(m_launch_entity.path_to_exec()); }

    void set_title(const std::string& title)
    {
        if (!m_label) {
            return;
        }
        m_label->set_text(title);
    }

    FastLaunchEntity& entity() { return m_launch_entity; }
    const FastLaunchEntity& entity() const { return m_launch_entity; }

private:
    void launch(const std::string& path_to_exec)
    {
        if (fork() == 0) {
            execve(path_to_exec.c_str(), 0, 0);
            std::abort();
        }
    }

    UI::Label* m_label;
    FastLaunchEntity m_launch_entity;
};