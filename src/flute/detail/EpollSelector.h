//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_DETAIL_EPOLL_SELECTOR_H
#define FLUTE_DETAIL_EPOLL_SELECTOR_H

#include <flute/Logger.h>
#include <flute/Selector.h>
#include <flute/flute-config.h>
#include <flute/socket_ops.h>
#include <sys/epoll.h>

#include <cerrno>
#include <cstring>
#include <vector>

namespace flute {
namespace detail {

static inline int create_epoll() {
    int result = FLUTE_INVALID_SOCKET;
#if defined(FLUTE_HAVE_EPOLL_CREATE1) && defined(EPOLL_CLOEXEC)
    result = ::epoll_create1(EPOLL_CLOEXEC);
#endif
    if (result == FLUTE_INVALID_SOCKET) {
        result = ::epoll_create(1024);
        if (result == FLUTE_INVALID_SOCKET) {
            auto error = getLastError();
            LOG_FATAL << "epoll_create error " << error << ": " << formatErrorString(error);
            ::exit(-1);
        }
        setSocketCloseOnExec(result);
    }
    return result;
}

class EpollSelector : public Selector {
public:
    EpollSelector() : m_descriptor(create_epoll()), m_events(INIT_EVENT_SIZE) {}
    ~EpollSelector() final {
        if (m_descriptor != FLUTE_INVALID_SOCKET) {
            flute::close(m_descriptor);
            m_descriptor = FLUTE_INVALID_SOCKET;
        }
    }

    void addEvent(socket_type descriptor, int old, int events, void* data) override {
        epoll_event ev{};
        ev.data.ptr = data;
        auto temp = old | events;
        if (temp & SelectorEvent::EVENT_READ) ev.events |= EPOLLIN;
        if (temp & SelectorEvent::EVENT_WRITE) ev.events |= EPOLLOUT;
        if (temp & SelectorEvent::EVENT_ET) ev.events |= EPOLLET;
        auto op = old == SelectorEvent::EVENT_NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
        auto ret = ::epoll_ctl(m_descriptor, op, descriptor, &ev);
        if (ret == -1) {
            auto error = getLastError();
            LOG_ERROR << "epoll_ctl error " << error << ":" << formatErrorString(error);
        }
    }

    void removeEvent(socket_type descriptor, int old, int events, void* data) override {
        epoll_event ev{};
        ev.data.ptr = data;
        auto temp = old & (~events);
        if (temp & SelectorEvent::EVENT_READ) ev.events |= EPOLLIN;
        if (temp & SelectorEvent::EVENT_WRITE) ev.events |= EPOLLOUT;
        if (temp & SelectorEvent::EVENT_ET) ev.events |= EPOLLET;
        auto op = temp == SelectorEvent::EVENT_NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
        auto ret = ::epoll_ctl(m_descriptor, op, descriptor, &ev);
        if (ret == -1) {
            auto error = getLastError();
            LOG_ERROR << "epoll_ctl error " << error << ":" << formatErrorString(error);
        }
    }

    int select(std::vector<SelectorEvent>& events, int timeout) override {
        auto count = ::epoll_wait(m_descriptor, m_events.data(), m_events.size(), timeout);
        if (count == -1) {
            // auto error = getLastError();
            // LOG_ERROR << "epoll_wait error " << error << ":" << formatErrorString(error);
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
            if (ev.events & (EPOLLHUP | EPOLLERR | EPOLLIN | EPOLLRDHUP)) e.events |= SelectorEvent::EVENT_READ;
            if (ev.events & EPOLLOUT) e.events |= SelectorEvent::EVENT_WRITE;
        }
        if (static_cast<std::size_t>(count) == m_events.size() && count < MAX_EVENT_SIZE) {
            m_events.resize(m_events.size() << 1);
        }
        return count;
    }

private:
    int m_descriptor;
    std::vector<epoll_event> m_events;
};

} // namespace detail
} // namespace flute

#endif // FLUTE_DETAIL_EPOLL_SELECTOR_H
