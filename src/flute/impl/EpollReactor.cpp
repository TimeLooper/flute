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

#include <flute/Logger.h>
#include <flute/socket_ops.h>
#include <sys/epoll.h>
#include <flute/EventLoop.h>

#include <cerrno>
#include <cstring>

namespace flute {
namespace impl {

static const int INIT_EVENT_SIZE = 32;
static const int MAX_EVENT_SIZE = 4096;

EpollReactor::EpollReactor() : m_epfd(INVALID_SOCKET), m_events() {
    m_events.resize(INIT_EVENT_SIZE);
    open();
}

EpollReactor::~EpollReactor() {
    close();
}

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
    auto ret = ::epoll_wait(m_epfd, m_events.data(), m_events.size(), timeout);
    if (ret == -1) {
        LOG_ERROR << "epoll_wait error " << errno << ": " << std::strerror(errno);
    }
    if (ret > 0 && static_cast<std::size_t>(ret) > events.size()) {
        events.resize(ret);
    }
    for (auto i = 0; i < ret; ++i) {
        auto& e = m_events[i];
        auto& fe = events[i];
        fe.data = e.data.ptr;
        fe.events = 0;
        if (e.events & EPOLLIN) {
            fe.events |= FileEvent::READ;
        }
        if (e.events & EPOLLOUT) {
            fe.events |= FileEvent::WRITE;
        }
    }
    if (ret > 0 && static_cast<std::size_t>(ret) == m_events.size() && m_events.size() < MAX_EVENT_SIZE) {
        m_events.resize(m_events.size() << 1);
    }
    return ret;
}

void EpollReactor::open() {
#if defined(FLUTE_HAVE_EPOLL_CREATE1) && defined(EPOLL_CLOEXEC)
    m_epfd = epoll_create1(EPOLL_CLOEXEC);
#endif
    if (m_epfd != INVALID_SOCKET) {
        return;
    }
    m_epfd = epoll_create(1024);
    if (m_epfd == INVALID_SOCKET) {
        LOG_FATAL << "epoll_create()";
        exit(-1);
    }
    setSocketCloseOnExec(m_epfd);
}

void EpollReactor::close() {
    if (m_epfd != INVALID_SOCKET) {
        flute::close(m_epfd);
        m_epfd = INVALID_SOCKET;
    }
}

} // namespace impl
} // namespace flute

#endif // FLUTE_HAVE_EPOLL