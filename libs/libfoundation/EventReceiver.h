/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include "Event.h"
#include <memory.h>
#include <syscalls.h>

namespace LFoundation {

class EventReceiver {
public:
    EventReceiver() { }
    ~EventReceiver() { }
    virtual void receive_event(UniquePtr<Event> event) {}

private:
};

}