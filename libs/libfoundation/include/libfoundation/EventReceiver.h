/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libfoundation/Event.h>
#include <memory>

namespace LFoundation {

class EventReceiver {
public:
    EventReceiver() = default;
    ~EventReceiver() = default;
    virtual void receive_event(std::unique_ptr<Event> event) { }

private:
};

} // namespace LFoundation