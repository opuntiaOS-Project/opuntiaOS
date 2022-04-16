/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <functional>
#include <libfoundation/Event.h>
#include <libfoundation/EventLoop.h>
#include <libfoundation/EventReceiver.h>
#include <libui/Event.h>
#include <vector>

namespace UI {
class GestureRecognizer;
}

// TODO: This invoker is really similar to what we can see in Control.h,
//       maybe move out them to Inovokers.h or smth like that?

namespace GestureDetails {
typedef std::function<void(const UI::GestureRecognizer*)> target_func_type;

class CallEvent final : public UI::Event {
public:
    friend class Caller;

    CallEvent(target_func_type callback, const UI::GestureRecognizer* recon)
        : Event(Event::Type::UIHandlerInvoke)
        , m_callback(callback)
        , m_recognizer(recon)
    {
    }
    ~CallEvent() = default;

    const UI::GestureRecognizer* recognizer() const { return m_recognizer; }

private:
    const UI::GestureRecognizer* m_recognizer;
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
        switch (event->type()) {
        case UI::Event::Type::UIHandlerInvoke: {
            CallEvent& own_event = *(CallEvent*)event.get();
            own_event.m_callback(own_event.recognizer());
            break;
        }
        }
    }
};
} // namespace details

namespace UI {

class GestureRecognizer {
public:
    enum State {
        Possible,
        Began,
        Cancelled,
        Ended,
    };

    GestureRecognizer(GestureDetails::target_func_type target)
        : m_target(target)
    {
    }
    virtual ~GestureRecognizer() = default;

    State state() const { return m_state; }
    virtual void mouse_up() { }
    virtual void mouse_down(const LG::Point<int>&) { }
    virtual void mouse_moved(const LG::Point<int>&) { }

protected:
    void update_state(State newstate)
    {
        m_state = newstate;
        did_update_state();
    }

    void did_update_state()
    {
        LFoundation::EventLoop::the().add(m_caller, new GestureDetails::CallEvent(m_target, this));
    }

private:
    State m_state { State::Ended };
    GestureDetails::target_func_type m_target;
    GestureDetails::Caller m_caller {};
};

class SwipeGestureRecognizer final : public GestureRecognizer {
public:
    SwipeGestureRecognizer(GestureDetails::target_func_type target)
        : GestureRecognizer(target)
    {
    }
    ~SwipeGestureRecognizer() = default;

    virtual void mouse_up() override
    {
        if (m_was_moved && m_was_down) {
            update_state(State::Ended);
        } else {
            update_state(State::Cancelled);
        }
        m_was_down = false;
        m_was_moved = false;
    }

    virtual void mouse_down(const LG::Point<int>&) override
    {
        m_was_down = true;
        update_state(State::Possible);
    }

    virtual void mouse_moved(const LG::Point<int>&) override
    {
        if (state() == State::Possible && m_was_down) {
            update_state(State::Began);
            m_was_moved = true;
        }
    }

private:
    bool m_was_moved { false };
    bool m_was_down { false };
};

} // namespace UI