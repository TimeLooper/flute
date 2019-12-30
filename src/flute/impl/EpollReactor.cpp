/*************************************************************************
 *
 * File Name:  EpollReactor.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#include <flute/impl/EpollReactor.h>
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

EpollReactor::EpollReactor() : m_descriptor(FLUTE_INVALID_SOCKET), m_events(INIT_EVENT_SIZE) { open(); }

EpollReactor::~EpollReactor() { close(); }

void EpollReactor::add(socket_type fd, int old, int event, void* data) {
    epoll_event ev{};
    ev.data.ptr = data;
    auto temp = old | event;
    ev.events = EPOLLET;
    if (temp & FileEvent::READ) ev.events |= EPOLLIN;
    if (temp & FileEvent::WRITE) ev.events |= EPOLLOUT;
    auto op = old == FileEvent::NONE ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    auto ret = ::epoll_ctl(m_descriptor, op, fd, &ev);
    if (ret == -1) {
        LOG_ERROR << "epoll_ctl error " << errno << ": " << std::strerror(errno);
    }
}

void EpollReactor::remove(socket_type fd, int old, int event, void* data) {
    epoll_event ev{};
    ev.data.ptr = data;
    auto temp = old & (~event);
    if (temp & (FileEvent::READ | FileEvent::WRITE)) ev.events |= EPOLLET;
    if (temp & FileEvent::READ) ev.events |= EPOLLIN;
    if (temp & FileEvent::WRITE) ev.events |= EPOLLOUT;
    auto op = temp == FileEvent::NONE ? EPOLL_CTL_DEL : EPOLL_CTL_MOD;
    auto ret = ::epoll_ctl(m_descriptor, op, fd, &ev);
    if (ret == -1) {
        LOG_ERROR << "epoll_ctl error " << errno << ": " << std::strerror(errno);
    }
}

int EpollReactor::wait(std::vector<FileEvent>& events, int timeout) {
    auto ret = ::epoll_wait(m_descriptor, m_events.data(), m_events.size(), timeout);
    if (ret == -1) {
        LOG_ERROR << "epoll_wait error " << errno << ": " << std::strerror(errno);
        return ret;
    }
    if (ret > 0 && static_cast<std::size_t>(ret) > events.size()) {
        events.resize(ret);
    }
    for (auto i = 0; i < ret; ++i) {
        auto& e = m_events[i];
        auto& temp = events[i];
        temp.data = e.data.ptr;
        temp.events = 0;
        if (e.events & EPOLLIN) {
            temp.events |= FileEvent::READ;
        }
        if (e.events & EPOLLOUT) {
            temp.events |= FileEvent::WRITE;
        }
    }

    if (ret > 0 && static_cast<std::size_t>(ret) == m_events.size() && m_events.size() < MAX_EVENT_SIZE) {
        m_events.resize(m_events.size() << 1);
    }
    return events.size();
}

void EpollReactor::open() {
#if defined(FLUTE_HAVE_EPOLL_CREATE1) && defined(EPOLL_CLOEXEC)
    m_descriptor = ::epoll_create1(EPOLL_CLOEXEC);
#endif
    if (m_descriptor == FLUTE_INVALID_SOCKET) {
        m_descriptor = ::epoll_create(1024);
        if (m_descriptor == FLUTE_INVALID_SOCKET) {
            LOG_FATAL << "epoll_create error " << errno << ": " << std::strerror(errno);
            ::exit(-1);
        }
        setSocketCloseOnExec(m_descriptor);
    }
}

void EpollReactor::close() {
    if (m_descriptor != FLUTE_INVALID_SOCKET) {
        flute::close(m_descriptor);
        m_descriptor = FLUTE_INVALID_SOCKET;
    }
}

} // namespace impl
} // namespace flute

#endif // FLUTE_HAVE_EPOLL