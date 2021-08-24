/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libfoundation/Event.h>
#include <libfoundation/EventLoop.h>
#include <libfoundation/EventReceiver.h>
#include <libui/View.h>
#include <memory>
#include <sys/types.h>

namespace UI {

class BaseViewController : public LFoundation::EventReceiver {
public:
    BaseViewController() = default;
    virtual ~BaseViewController() = default;

    virtual void view_did_load() { }
    virtual void receive_event(std::unique_ptr<LFoundation::Event> event) override
    {
        if (event->type() == Event::Type::ViewDidLoad) {
            view_did_load();
        }
    }

private:
};

template <class ViewT>
class ViewController : public BaseViewController {
public:
    ViewController(ViewT& view)
        : m_view(view)
    {
    }
    virtual ~ViewController() = default;

    ViewT& view() { return m_view; }
    const ViewT& view() const { return m_view; }

private:
    ViewT& m_view;
};

} // namespace UI