#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/Window.h>

#define FRAME_COLOR 0x003DFF
#define BODY_COLOR 0x819EFA

int main(int argc, char** argv)
{
    auto& app = create<UI::App>();
    auto& window = create<UI::Window>(165, 140, "/res/icons/apps/about.icon");
    auto& superview = window.create_superview<UI::View>();
    superview.set_background_color(LG::Color(BODY_COLOR));

    auto& button = superview.add_subview<UI::Button>(LG::Rect(35, 75, 10, 10));
    button.set_background_color(LG::Color(FRAME_COLOR));
    button.set_title("Just Button");

    auto& label = superview.add_subview<UI::Label>(LG::Rect(35, 45, 10, 10));
    label.set_text_color(LG::Color::White);
    label.set_text("Alive in oneOS!");
    label.set_width(label.preferred_width());

    window.superview()->set_needs_display();
    window.set_title("About");
    window.set_frame_style(LG::Color(FRAME_COLOR));
    return app.run();
}