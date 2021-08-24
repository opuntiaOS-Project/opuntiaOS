/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <functional>
#include <libfoundation/Event.h>
#include <libfoundation/EventReceiver.h>
#include <libg/Context.h>
#include <libui/Event.h>
#include <libui/View.h>
#include <libui/Window.h>

typedef std::function<void(UI::View*)> target_func_type;

namespace details {

class CallEvent final : public UI::Event {
public:
    friend class Caller;

    CallEvent(target_func_type callback, UI::View* view)
        : Event(Event::Type::UIHandlerInvoke)
        , m_callback(callback)
        , m_view(view)
    {
    }
    ~CallEvent() = default;

    UI::View* view() const { return m_view; }

private:
    UI::View* m_view;
    target_func_type m_callback;
};

class Caller : public LFoundation::EventReceiver {
public:
    friend class EventLoop;

    Caller()
        : EventReceiver()
    {
    }

    void receive_event(std::unique_ptr<LFoundation::Event> event) override
    {
        if (event->type() == UI::Event::Type::UIHandlerInvoke) {
            CallEvent& own_event = *(CallEvent*)event.get();
            own_event.m_callback(own_event.view());
        }
    }
};
} // namespace details

namespace UI {
class Control : public View {
    UI_OBJECT();

public:
    struct Target {
        target_func_type target_func;
        UI::Event::Type for_event;
    };

    ~Control() = default;

    template <class ActionType>
    void add_target(ActionType target_func, UI::Event::Type for_event)
    {
        m_targets.push_back(Target { target_func, for_event });
    }

    void send_actions(UI::Event::Type for_event)
    {
        auto& event_loop = LFoundation::EventLoop::the();
        for (auto& target : m_targets) {
            if (target.for_event == for_event) {
                event_loop.add(m_caller, new details::CallEvent(target.target_func, this));
            }
        }
    }

    std::vector<Target>& targets() { return m_targets; }
    const std::vector<Target>& targets() const { return m_targets; }

protected:
    Control(View* superview, const LG::Rect& r)
        : View(superview, r)
    {
    }
    Control(View* superview, Window* window, const LG::Rect& r) = delete;

private:
    std::vector<Target> m_targets {};
    bool is_selected { false };
    bool is_enabled { false };
    details::Caller m_caller {};
};
} // namespace UI