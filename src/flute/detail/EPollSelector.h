//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_EPOLL_REACTOR_H
#define FLUTE_EPOLL_REACTOR_H

#include <flute/flute-config.h>

#include <flute/Logger.h>
#include <flute/Selector.h>
#include <flute/socket_ops.h>

#include <cerrno>
#include <cstring>
#include <vector>

#include <sys/epoll.h>

namespace flute {
namespace detail {

class EPollSelector : public Selector {
public:
    EPollSelector();
    ~EPollSelector() final;

    void addEvent(socket_type descriptor, int old, int events, void* data) override;

    void removeEvent(socket_type descriptor, int old, int events, void* data) override;

    int wait(std::vector<FluteEvent>& events, int timeout) override;

private:
    int m_descriptor;
    std::vector<epoll_event> m_events;
};

inline int create_epoll() {
    int result = FLUTE_INVALID_SOCKET;
#if defined(FLUTE_HAVE_EPOLL_CREATE1) && defined(EPOLL_CLOEXEC)
    result = ::epoll_create1(EPOLL_CLOEXEC);
#endif
    if (result == FLUTE_INVALID_SOCKET) {
        result = ::epoll_create(1024);
        if (result == FLUTE_INVALID_SOCKET) {
            LOG_FATAL << "epoll_create error " << errno << ": " << std::strerror(errno);
            ::exit(-1);
        }
        setSocketCloseOnExec(result);
    }
    return result;
}

inline EPollSelector::EPollSelector() : m_descriptor(create_epoll()), m_events(INIT_EVENT_SIZE) {}

inline EPollSelector::~EPollSelector() {
    if (m_descriptor != FLUTE_INVALID_SOCKET) {
        flute::close(m_descriptor);
        m_descriptor = FLUTE_INVALID_SOCKET;
    }
}

inline void EPollSelector::addEvent(socket_type descriptor, int old, int events, void* data) {
    epoll_event ev{};
    ev.data.ptr = data;
    auto temp = old | events;
    if (temp & FluteEvent::READ) ev.events |= EPOLLIN;
    if (temp & FluteEvent::WRITE) ev.events |= EPOLLOUT;
    auto op = old == FluteEvent::NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    auto ret = ::epoll_ctl(m_descriptor, op, descriptor, &ev);
    if (ret == -1) {
        LOG_ERROR << "epoll_ctl error " << errno << ":" << std::strerror(errno);
    }
}

inline void EPollSelector::removeEvent(socket_type descriptor, int old, int events, void* data) {
    epoll_event ev{};
    ev.data.ptr = data;
    auto temp = old & (~events);
    if (temp & FluteEvent::READ) ev.events |= EPOLLIN;
    if (temp & FluteEvent::WRITE) ev.events |= EPOLLOUT;
    auto op = temp == FluteEvent::NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    auto ret = ::epoll_ctl(m_descriptor, op, descriptor, &ev);
    if (ret == -1) {
        LOG_ERROR << "epoll_ctl error " << errno << ":" << std::strerror(errno);
    }
}

inline int EPollSelector::wait(std::vector<FluteEvent>& events, int timeout) {
    auto count = ::epoll_wait(m_descriptor, m_events.data(), m_events.size(), timeout);
    if (count == -1) {
        LOG_ERROR << "epoll_wait error " << errno << ":" << std::strerror(errno);
        return -1;
    }
    if (count > 0 && static_cast<std::size_t>(count) > events.size()) {
        events.resize(count);
    }
    for (auto i = 0; i < count; ++i) {
        auto& ev = m_events[i];
        auto& e = events[i];
        e.data = ev.data.ptr;
        e.events = 0;
        if (ev.events & EPOLLIN) {
            e.events |= FluteEvent::READ;
        }
        if (ev.events & EPOLLOUT) {
            e.events |= FluteEvent::WRITE;
        }
    }
    if (static_cast<std::size_t>(count) == m_events.size() && count < MAX_EVENT_SIZE) {
        m_events.resize(m_events.size() << 1);
    }
    return 0;
}

} // namespace detail
} // namespace flute

#endif // FLUTE_EPOLL_REACTOR_H
