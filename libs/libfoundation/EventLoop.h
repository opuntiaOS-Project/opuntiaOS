/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "Event.h"
#include "EventReceiver.h"
#include "Receivers.h"
#include <memory.h>
#include <std/Vector.h>
#include <syscalls.h>

namespace LFoundation {

class QueuedEvent {
public:
    friend class EventLoop;
    QueuedEvent(EventReceiver& rec, Event* ptr)
        : event(ptr)
        , receiver(rec)
    {
    }

    QueuedEvent(QueuedEvent&& qe)
        : event(move(qe.event))
        , receiver(qe.receiver)
    {
    }

    QueuedEvent& operator=(QueuedEvent&& qe)
    {
        event = move(qe.event);
        receiver = qe.receiver;
        return *this;
    }

    ~QueuedEvent() = default;

    EventReceiver& receiver;
    UniquePtr<Event> event { nullptr };
};

class EventLoop {
public:
    static EventLoop& the();
    EventLoop();

    inline void add(int fd, void (*on_read)(), void (*on_write)())
    {
        m_waiting_fds.push_back(FDWaiter(fd, on_read, on_write));
    }

    inline void add(const Timer& timer)
    {
        m_timers.push_back(timer);
    }

    inline void add(Timer&& timer)
    {
        m_timers.push_back(move(timer));
    }

    inline void add(EventReceiver& rec, Event* ptr)
    {
        m_event_queue.push_back(QueuedEvent(rec, ptr));
    }

    inline void stop(int exit_code) { m_exit_code = exit_code, m_stop_flag = true; }
    void check_fds();
    void check_timers();
    void pump();
    int run();

private:
    bool m_stop_flag { false };
    int m_exit_code { 0 };
    Vector<FDWaiter> m_waiting_fds;
    Vector<Timer> m_timers;
    Vector<QueuedEvent> m_event_queue;
};
}