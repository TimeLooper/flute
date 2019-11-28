/*************************************************************************
 *
 * File Name:  KqueueReactor.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/29 00:30:07
 *
 *************************************************************************/

#include <flute/impl/KqueueReactor.h>
#include <flute/Logger.h>
#include <flute/socket_ops.h>
#include <flute/EventLoop.h>

#include <cerrno>
#include <cstring>
#include <ctime>

namespace flute {
namespace impl {

static const int INIT_EVENT_SIZE = 32;
static const int MAX_EVENT_SIZE = 4096;

KqueueReactor::KqueueReactor() : m_kqfd(INVALID_SOCKET), m_events() {
    m_events.resize(INIT_EVENT_SIZE);
    open();
}

KqueueReactor::~KqueueReactor() {
    close();
}

void KqueueReactor::add(socket_type fd, int old, int event, void* data) {
    struct timespec now;
    now.tv_nsec = now.tv_sec = 0;
    struct kevent ev[2];
    auto n = 0;
    if (event & FileEvent::READ) {
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_ADD, 0, 0, data);
    }
    if (event & FileEvent::WRITE) {
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_ADD, 0, 0, data);
    }
    auto ret = kevent(m_kqfd, ev, n, nullptr, 0, &now);
    if (ret != 0) {
        LOG_ERROR << "kevent error " << errno << ": " << std::strerror(errno);
    }
}

void KqueueReactor::remove(socket_type fd, int old, int event, void* data) {
    struct timespec now;
    now.tv_nsec = now.tv_sec = 0;
    struct kevent ev[2];
    auto n = 0;
    if (event & FileEvent::READ) {
        EV_SET(&ev[n++], fd, EVFILT_READ, EV_DELETE, 0, 0, data);
    }
    if (event & FileEvent::WRITE) {
        EV_SET(&ev[n++], fd, EVFILT_WRITE, EV_DELETE, 0, 0, data);
    }
    auto ret = kevent(m_kqfd, ev, n, nullptr, 0, &now);
    if (ret != 0) {
        LOG_ERROR << "kevent error " << errno << ": " << std::strerror(errno);
    }
}

int KqueueReactor::wait(std::vector<FileEvent>& events, int timeout) {
    int ret = 0;
    if (timeout > 0) {
        struct timespec timeoutSpec;
        timeoutSpec.tv_sec = timeout / 1000;
        timeoutSpec.tv_nsec = (timeout % 1000) * 1000 * 1000;
        ret = ::kevent(m_kqfd, nullptr, 0, m_events.data(), m_events.size(), &timeoutSpec);
    } else {
        ret = ::kevent(m_kqfd, nullptr, 0, m_events.data(), m_events.size(), nullptr);
    }
    if (ret > 0 && ret > events.size()) {
        events.resize(ret);
    }
    for (auto i = 0; i < ret; ++i) {
        auto& ev = events[i];
        auto& kev = m_events[i];
        ev.data = kev.udata;
        ev.events = 0;
        if (kev.filter & EVFILT_READ) {
            ev.events |= FileEvent::READ;
        }
        if (kev.filter & EVFILT_WRITE) {
            ev.events |= FileEvent::WRITE;
        }
    }
    if (ret > 0 && m_events.size() == ret && m_events.size() < MAX_EVENT_SIZE) {
        m_events.resize(m_events.size() << 1);
    }
    return ret;
}

void KqueueReactor::open() {
    m_kqfd = kqueue();
    if (m_kqfd < 0) {
        LOG_FATAL << "kqueue init failed " << errno << ": " << std::strerror(errno);
        exit(-1);
    }
}

void KqueueReactor::close() {
    flute::close(m_kqfd);
    m_kqfd = INVALID_SOCKET;
}

} // namespace impl
} // namespace flute