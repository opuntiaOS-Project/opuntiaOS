/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cstring>
#include <ctime>
#include <iostream>
#include <libfoundation/EventLoop.h>
#include <libfoundation/Logger.h>
#include <memory>
#include <sched.h>
#include <sys/select.h>
#include <sys/time.h>
#include <unistd.h>

namespace LFoundation {

static EventLoop* s_the = nullptr;

EventLoop& EventLoop::the()
{
    return *s_the;
}

EventLoop::EventLoop()
{
    s_the = this;
}

void EventLoop::check_fds()
{
    if (m_waiting_fds.size() == 0) {
        return;
    }
    fd_set_t readfds;
    fd_set_t writefds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    int nfds = -1;
    for (int i = 0; i < m_waiting_fds.size(); i++) {
        if (m_waiting_fds[i].m_on_read) {
            FD_SET(m_waiting_fds[i].m_fd, &readfds);
        }
        if (m_waiting_fds[i].m_on_write) {
            FD_SET(m_waiting_fds[i].m_fd, &writefds);
        }
        if (nfds < m_waiting_fds[i].m_fd) {
            nfds = m_waiting_fds[i].m_fd;
        }
    }

    // For now, that means, that we don't wait for fds.
    timeval_t timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;

    int res = select(nfds + 1, &readfds, &writefds, nullptr, &timeout);

    for (int i = 0; i < m_waiting_fds.size(); i++) {
        if (m_waiting_fds[i].m_on_read) {
            if (FD_ISSET(m_waiting_fds[i].m_fd, &readfds)) {
                m_event_queue.push_back(QueuedEvent(m_waiting_fds[i], new FDWaiterReadEvent()));
            }
        }
        if (m_waiting_fds[i].m_on_write) {
            if (FD_ISSET(m_waiting_fds[i].m_fd, &writefds)) {
                m_event_queue.push_back(QueuedEvent(m_waiting_fds[i], new FDWaiterWriteEvent()));
            }
        }
    }
}

void EventLoop::check_timers()
{
    if (m_timers.empty()) {
        return;
    }

    std::timespec tp;
    clock_gettime(CLOCK_MONOTONIC, &tp);

    for (auto& timer : m_timers) {
        if (!timer.expired(tp)) {
            continue;
        }

        m_event_queue.push_back(QueuedEvent(timer, new TimerEvent()));

        if (timer.repeated()) {
            timer.reload(tp);
        }
    }
}

[[gnu::flatten]] void EventLoop::pump()
{
    check_fds();
    check_timers();
    std::vector<QueuedEvent> events_to_dispatch(std::move(m_event_queue));
    m_event_queue.clear();
    for (auto& event : events_to_dispatch) {
        event.receiver.receive_event(std::move(event.event));
    }

    if (!events_to_dispatch.size()) {
        sched_yield();
    }
}

int EventLoop::run()
{
    while (!m_stop_flag) {
        pump();
    }
    return m_exit_code;
}

} // namespace LFoundation