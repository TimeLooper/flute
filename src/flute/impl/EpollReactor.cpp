/*************************************************************************
 *
 * File Name:  EpollReactor.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#include "EpollReactor.h"
#ifdef FLUTE_HAVE_EPOLL

#include <flute/EventLoop.h>
#include <flute/Logger.h>
#include <flute/socket_ops.h>
#include <sys/epoll.h>

#include <cerrno>
#include <cstring>

namespace flute {
namespace impl {

static const int INIT_EVENT_SIZE = 32;
static const int MAX_EVENT_SIZE = 4096;

EpollReactor::EpollReactor()
    : m_epfd(FLUTE_INVALID_SOCKET)
#ifdef USING_TIMERFD
    , m_timerfd(FLUTE_INVALID_SOCKET)
#endif
    , m_events(INIT_EVENT_SIZE) {
    open();
}

EpollReactor::~EpollReactor() { close(); }

void EpollReactor::add(socket_type fd, int old, int event, void* data) {
    epoll_event ev{};
    ev.data.ptr = data;
    auto temp = old | event;
    if (temp & FileEvent::READ) ev.events |= EPOLLIN;
    if (temp & FileEvent::WRITE) ev.events |= EPOLLOUT;
    auto op = old == FileEvent::NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    auto ret = ::epoll_ctl(m_epfd, op, fd, &ev);
    if (ret == -1) {
        LOG_ERROR << "epoll_ctl error " << errno << ": " << std::strerror(errno);
    }
}

void EpollReactor::remove(socket_type fd, int old, int event, void* data) {
    epoll_event ev{};
    ev.data.ptr = data;
    auto temp = old & (~event);
    if (temp & FileEvent::READ) ev.events |= EPOLLIN;
    if (temp & FileEvent::WRITE) ev.events |= EPOLLOUT;
    auto op = temp == FileEvent::NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    auto ret = ::epoll_ctl(m_epfd, op, fd, &ev);
    if (ret == -1) {
        LOG_ERROR << "epoll_ctl error " << errno << ": " << std::strerror(errno);
    }
}

int EpollReactor::wait(std::vector<FileEvent>& events, int timeout) {
    auto epoll_timeout = timeout;
#ifdef USING_TIMERFD
    if (m_timerfd != FLUTE_INVALID_SOCKET && timeout > 0) {
        struct itimerspec newValue {};
        newValue.it_value.tv_sec = timeout / 1000;
        newValue.it_value.tv_nsec = (timeout % 1000) * 1000;
        auto ret = ::timerfd_settime(m_timerfd, 0, &newValue, nullptr);
        if (ret != 0) {
            LOG_ERROR << "timerfd_settime error " << errno << ": " << std::strerror(errno);
        } else {
            epoll_timeout = -1;
        }
    }
#endif
    auto ret = ::epoll_wait(m_epfd, m_events.data(), m_events.size(), epoll_timeout);
    if (ret == -1) {
        LOG_ERROR << "epoll_wait error " << errno << ": " << std::strerror(errno);
    }
    if (ret > 0 && static_cast<std::size_t>(ret) > events.size()) {
        events.resize(ret);
    }
    auto cnt = 0;
    for (auto i = 0, cnt = 0; i < ret; ++i) {
        auto& e = m_events[i];
#ifdef USING_TIMERFD
        // timerfd
        if (e.data.ptr == &m_timerfd) {
            std::uint64_t num;
            flute::read(m_timerfd, &num, sizeof(num));
            continue;
        }
#endif
        auto& fe = events[cnt];
        fe.data = e.data.ptr;
        fe.events = 0;
        if (e.events & EPOLLIN) {
            fe.events |= FileEvent::READ;
        }
        if (e.events & EPOLLOUT) {
            fe.events |= FileEvent::WRITE;
        }
        cnt += 1;
    }
    
    if (ret > 0 && static_cast<std::size_t>(ret) == m_events.size() && m_events.size() < MAX_EVENT_SIZE) {
        m_events.resize(m_events.size() << 1);
    }
    return cnt;
}

void EpollReactor::open() {
#if defined(FLUTE_HAVE_EPOLL_CREATE1) && defined(EPOLL_CLOEXEC)
    m_epfd = ::epoll_create1(EPOLL_CLOEXEC);
#endif
    if (m_epfd == FLUTE_INVALID_SOCKET) {
        m_epfd = ::epoll_create(1024);
        if (m_epfd == FLUTE_INVALID_SOCKET) {
            LOG_FATAL << "epoll_create error " << errno << ": " << std::strerror(errno);
            ::exit(-1);
        }
        setSocketCloseOnExec(m_epfd);
    }
#ifdef USING_TIMERFD
    m_timerfd = ::timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
    if (m_timerfd == FLUTE_INVALID_SOCKET) {
        LOG_WARN << "timerfd_create error " << errno << ": " << std::strerror(errno);
    } else {
        add(m_timerfd, FileEvent::NONE, FileEvent::READ, &m_timerfd);
    }
#endif
}

void EpollReactor::close() {
    if (m_epfd != FLUTE_INVALID_SOCKET) {
        flute::close(m_epfd);
        m_epfd = FLUTE_INVALID_SOCKET;
    }
#ifdef USING_TIMERFD
    if (m_timerfd != FLUTE_INVALID_SOCKET) {
        flute::close(m_timerfd);
        remove(m_timerfd, FileEvent::READ, FileEvent::READ, &m_timerfd);
        m_timerfd = FLUTE_INVALID_SOCKET;
    }
#endif
}

} // namespace impl
} // namespace flute

#endif // FLUTE_HAVE_EPOLL