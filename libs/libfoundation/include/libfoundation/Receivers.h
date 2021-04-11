/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libfoundation/Event.h>
#include <libfoundation/EventReceiver.h>
#include <memory>
#include <vector>

namespace LFoundation {

class FDWaiterReadEvent final : public Event {
public:
    FDWaiterReadEvent()
        : Event(Event::Type::FdWaiterRead)
    {
    }
    ~FDWaiterReadEvent() = default;
};

class FDWaiterWriteEvent final : public Event {
public:
    FDWaiterWriteEvent()
        : Event(Event::Type::FdWaiterWrite)
    {
    }
    ~FDWaiterWriteEvent() = default;
};

class FDWaiter : public EventReceiver {
public:
    friend class EventLoop;

    FDWaiter(int fd, std::function<void(void)> on_read, std::function<void(void)> on_write)
        : EventReceiver()
        , m_fd(fd)
        , m_on_read(on_read)
        , m_on_write(on_write)
    {
    }

    FDWaiter(FDWaiter&& fdw)
        : EventReceiver()
        , m_fd(fdw.m_fd)
        , m_on_read(fdw.m_on_read)
        , m_on_write(fdw.m_on_write)
    {
    }

    FDWaiter& operator=(const FDWaiter& fdw)
    {
        m_fd = fdw.m_fd;
        m_on_read = fdw.m_on_read;
        m_on_write = fdw.m_on_write;
        return *this;
    }

    FDWaiter& operator=(FDWaiter&& fdw)
    {
        m_fd = fdw.m_fd;
        m_on_read = fdw.m_on_read;
        m_on_write = fdw.m_on_write;
        return *this;
    }

    void receive_event(std::unique_ptr<Event> event) override
    {
        if (event->type() == Event::Type::FdWaiterRead) {
            m_on_read();
        } else if (event->type() == Event::Type::FdWaiterWrite) {
            m_on_write();
        }
    }

    inline int fd() const { return m_fd; }

private:
    int m_fd;
    std::function<void(void)> m_on_read;
    std::function<void(void)> m_on_write;
};

class TimerEvent final : public Event {
public:
    TimerEvent()
        : Event(Event::Type::DeferredInvoke)
    {
    }
    ~TimerEvent() = default;
};

class Timer : public EventReceiver {
public:
    friend class EventLoop;

    explicit Timer(std::function<void(void)> callback)
        : EventReceiver()
        , m_callback(callback)
    {
    }

    Timer(const Timer& fdw)
        : EventReceiver()
        , m_callback(fdw.m_callback)
    {
    }

    Timer(Timer&& fdw)
        : EventReceiver()
        , m_callback(fdw.m_callback)
    {
    }

    Timer& operator=(const Timer& fdw)
    {
        m_callback = fdw.m_callback;
        return *this;
    }

    Timer& operator=(Timer&& fdw)
    {
        m_callback = fdw.m_callback;
        return *this;
    }

    inline bool expired() { return true; } // FIXME

    void receive_event(std::unique_ptr<Event> event) override
    {
        m_callback();
    }

private:
    std::function<void(void)> m_callback;
};

class CallEvent final : public Event {
public:
    CallEvent(void (*callback)())
        : Event(Event::Type::DeferredInvoke)
        , m_callback(callback)
    {
    }
    ~CallEvent() = default;

private:
    void (*m_callback)();
};

// class Caller : public EventReceiver {
// public:
//     friend class EventLoop;

//     Caller()
//         : EventReceiver()
//     {
//     }

//     void receive_event(std::unique_ptr<Event> event) override
//     {
//         if (event->type() == Event::Type::DeferredInvoke) {

//         }
//     }
// };

} // namespace LFoundation