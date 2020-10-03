/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
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