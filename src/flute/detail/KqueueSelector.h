//
// Created by why on 2019/12/31.
//

#ifndef FLUTE_DETAIL_KQUEUE_SELECTOR_H
#define FLUTE_DETAIL_KQUEUE_SELECTOR_H

#include <flute/Logger.h>
#include <flute/Selector.h>
#include <flute/flute_types.h>
#include <flute/socket_ops.h>
#include <sys/event.h>

#include <cerrno>
#include <cstring>
#include <ctime>
#include <vector>

namespace flute {
namespace detail {

static inline socket_type createKqueue() {
    auto result = kqueue();
    if (result < 0) {
        auto error = getLastError();
        LOG_FATAL << "kqueue init failed " << error << ": " << formatErrorString(error);
        exit(-1);
    }
    return result;
}

class KqueueSelector : public Selector {
public:
    KqueueSelector() : m_descriptor(createKqueue()), m_events(INIT_EVENT_SIZE) {}
    ~KqueueSelector() final {
        flute::close(m_descriptor);
        m_descriptor = FLUTE_INVALID_SOCKET;
    }

    void addEvent(socket_type descriptor, int old, int events, void* data) override {
        struct timespec now;
        now.tv_nsec = now.tv_sec = 0;
        struct kevent ev[2];
        auto n = 0;
        if (events & SelectorEvent::EVENT_READ) {
            EV_SET(&ev[n++], descriptor, EVFILT_READ, EV_ADD, 0, 0, data);
        }
        if (events & SelectorEvent::EVENT_WRITE) {
            EV_SET(&ev[n++], descriptor, EVFILT_WRITE, EV_ADD, 0, 0, data);
        }
        auto ret = kevent(m_descriptor, ev, n, nullptr, 0, &now);
        if (ret != 0) {
            auto error = getLastError();
            LOG_ERROR << "kevent error " << error << ": " << formatErrorString(error);
        }
    }

    void removeEvent(socket_type descriptor, int old, int events, void* data) override {
        struct timespec now;
        now.tv_nsec = now.tv_sec = 0;
        struct kevent ev[2];
        auto n = 0;
        if (events & SelectorEvent::EVENT_READ) {
            EV_SET(&ev[n++], descriptor, EVFILT_READ, EV_DELETE, 0, 0, data);
        }
        if (events & SelectorEvent::EVENT_WRITE) {
            EV_SET(&ev[n++], descriptor, EVFILT_WRITE, EV_DELETE, 0, 0, data);
        }
        auto ret = kevent(m_descriptor, ev, n, nullptr, 0, &now);
        if (ret != 0) {
            auto error = getLastError();
            LOG_ERROR << "kevent error " << error << ": " << formatErrorString(error);
        }
    }

    int select(std::vector<SelectorEvent>& events, int timeout) override {
        int ret = 0;
        if (timeout > 0) {
            struct timespec timeoutSpec;
            timeoutSpec.tv_sec = timeout / 1000;
            timeoutSpec.tv_nsec = (timeout % 1000) * 1000 * 1000;
            ret = ::kevent(m_descriptor, nullptr, 0, m_events.data(), m_events.size(), &timeoutSpec);
        } else {
            ret = ::kevent(m_descriptor, nullptr, 0, m_events.data(), m_events.size(), nullptr);
        }
        if (ret == -1) {
            return -1;
        }
        if (ret > 0 && ret > events.size()) {
            events.resize(ret);
        }
        for (auto i = 0; i < ret; ++i) {
            auto& ev = events[i];
            auto& kev = m_events[i];
            ev.data = kev.udata;
            ev.events = 0;
            if (kev.flags & EV_ERROR) ev.events |= SelectorEvent::EVENT_READ;
            if (kev.filter == EVFILT_READ) ev.events |= SelectorEvent::EVENT_READ;
            if (kev.filter == EVFILT_WRITE) ev.events |= SelectorEvent::EVENT_WRITE;
        }
        if (ret > 0 && m_events.size() == ret && m_events.size() < MAX_EVENT_SIZE) {
            m_events.resize(m_events.size() << 1);
        }
        return ret;
    }

private:
    socket_type m_descriptor;
    std::vector<struct kevent> m_events;
};

} // namespace detail
} // namespace flute

#endif // FLUTE_DETAIL_KQUEUE_SELECTOR_H
