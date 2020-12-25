#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/Window.h>
#include <syscalls.h>

#define FRAME_COLOR 0x003DFF
#define BODY_COLOR 0x819EFA

int main(int argc, char** argv)
{
    new UI::App();
    auto* window_ptr = new UI::Window(165, 140, "/res/icons/apps/about.icon");
    window_ptr->set_superview(new UI::View(window_ptr->bounds()));
    window_ptr->superview()->set_background_color(LG::Color(BODY_COLOR));

    auto* button = new UI::Button({ 35, 75, 10, 10 });
    window_ptr->superview()->add_subview(button);
    button->set_background_color(LG::Color(FRAME_COLOR));
    button->set_title("Just Button");

    auto* label = new UI::Label({ 35, 45, 10, 10 });
    window_ptr->superview()->add_subview(label);
    label->set_text_color(LG::Color::White);
    label->set_text("Alive in oneOS!");
    label->set_width(label->preferred_width());

    window_ptr->superview()->set_needs_display();
    window_ptr->set_title("About");
    window_ptr->set_frame_style(LG::Color(FRAME_COLOR));
    return UI::App::the().run();
}