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
#include <memory.h>
#include <std/Function.h>
#include <std/Vector.h>
#include <syscalls.h>

namespace LFoundation {

class FDWaiterReadEvent final : public Event {
public:
    FDWaiterReadEvent()
        : Event(Event::Type::FdWaiterRead)
    {
    }
    ~FDWaiterReadEvent() { }
};

class FDWaiterWriteEvent final : public Event {
public:
    FDWaiterWriteEvent()
        : Event(Event::Type::FdWaiterWrite)
    {
    }
    ~FDWaiterWriteEvent() { }
};

class FDWaiter : EventReceiver {
public:
    friend class EventLoop;
    FDWaiter(int fd, Function<void()> on_read, Function<void()> on_write)
        : m_fd(fd)
        , m_on_read(on_read)
        , m_on_write(on_write)
    {
    }

    FDWaiter(FDWaiter&& fdw)
        : m_fd(fdw.m_fd)
        , m_on_read(fdw.m_on_read)
        , m_on_write(fdw.m_on_write)
    {
    }

    FDWaiter& operator=(FDWaiter&& fdw)
    {
        m_fd = fdw.m_fd;
        m_on_read = fdw.m_on_read;
        m_on_write = fdw.m_on_write;
        return *this;
    }

    void receive_event(UniquePtr<Event> event) override
    {
        if (event->type() == Event::Type::FdWaiterRead) {
            m_on_read();
        } else if (event->type() == Event::Type::FdWaiterWrite) {
            m_on_write();
        }
    }

    inline int fd() { return m_fd; }

private:
    int m_fd;
    Function<void()> m_on_read;
    Function<void()> m_on_write;
};

class QueuedEvent {
public:
    friend class EventLoop;
    QueuedEvent(EventReceiver& rec, UniquePtr<Event>&& ptr)
        : event(move(ptr))
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
        event = qe.event.release();
        receiver = qe.receiver;
        return *this;
    }

    EventReceiver& receiver;
    UniquePtr<Event> event;
};

class EventLoop {
public:
    static EventLoop& the();
    EventLoop();

    inline void add(int fd, Function<void()> on_read, Function<void()> on_write)
    {
        m_waiting_fds.push_back(FDWaiter(fd, on_read, on_write));
    }

    inline void add(EventReceiver& rec, UniquePtr<Event>&& ptr)
    {
        m_event_queue.push_back(QueuedEvent(rec, move(ptr)));
    }

    void check_fds();
    void pump();
    void run();

private:
    Vector<FDWaiter> m_waiting_fds;
    Vector<QueuedEvent> m_event_queue;
};
}