/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
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

    ~QueuedEvent() { }

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

    void check_fds();
    void check_timers();
    void pump();
    void run();

private:
    Vector<FDWaiter> m_waiting_fds;
    Vector<Timer> m_timers;
    Vector<QueuedEvent> m_event_queue;
};
}