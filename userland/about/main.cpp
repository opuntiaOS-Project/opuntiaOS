#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <memory>
#include <sys/utsname.h>

#define BODY_COLOR 0x819EFA

int main(int argc, char** argv)
{
    utsname_t uts;
    int rc = uname(&uts);

    auto& app = std::oneos::construct<UI::App>();
    auto& window = std::oneos::construct<UI::Window>(200, 140, "/res/icons/apps/about.icon");
    auto& superview = window.create_superview<UI::View>();
    superview.set_background_color(LG::Color::LightSystemBackground);

    auto& label = superview.add_subview<UI::Label>(LG::Rect(0, 0, 16, 16));
    label.set_text_color(LG::Color::LightSystemText);
    label.set_text("oneOS");
    label.set_font(LG::Font::system_bold_font());
    label.set_width(label.preferred_width());

    auto& target_label = superview.add_subview<UI::Label>(LG::Rect(0, 0, 16, 16));
    target_label.set_text_color(LG::Color::LightSystemText);
    target_label.set_text(LG::string("for ") + uts.machine);
    target_label.set_width(target_label.preferred_width());

    auto& version_label = superview.add_subview<UI::Label>(LG::Rect(0, 0, 16, 16));
    version_label.set_text_color(LG::Color::LightSystemText);
    version_label.set_text(uts.release);
    version_label.set_width(version_label.preferred_width());

    auto& button = superview.add_subview<UI::Button>(LG::Rect(0, 0, 10, 10));
    button.set_background_color(LG::Color::LightSystemButton);
    button.set_title("System info");
    button.set_title_color(LG::Color::LightSystemText);

    auto& footer = superview.add_subview<UI::Label>(LG::Rect(0, 0, 16, 16));
    footer.set_text_color(LG::Color::LightSystemText);
    footer.set_text("(c) 2020-2021");
    footer.set_width(footer.preferred_width());

    superview.add_constraint(UI::Constraint(label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
    superview.add_constraint(UI::Constraint(label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, UI::SafeArea::Top));

    superview.add_constraint(UI::Constraint(target_label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, label, UI::Constraint::Attribute::Right, 1, 8));
    superview.add_constraint(UI::Constraint(target_label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, label, UI::Constraint::Attribute::Top, 1, 0));

    superview.add_constraint(UI::Constraint(version_label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
    superview.add_constraint(UI::Constraint(version_label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, label, UI::Constraint::Attribute::Bottom, 1, 8));

    superview.add_constraint(UI::Constraint(button, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
    superview.add_constraint(UI::Constraint(button, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, version_label, UI::Constraint::Attribute::Bottom, 1, 8));

    superview.add_constraint(UI::Constraint(footer, UI::Constraint::Attribute::CenterX, UI::Constraint::Relation::Equal, superview, UI::Constraint::Attribute::CenterX, 1, 0));
    superview.add_constraint(UI::Constraint(footer, UI::Constraint::Attribute::Bottom, UI::Constraint::Relation::Equal, superview, UI::Constraint::Attribute::Bottom, 1, -UI::SafeArea::Bottom));

    window.superview()->set_needs_layout();
    window.set_title("About");
    return app.run();
}