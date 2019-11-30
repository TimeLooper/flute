/*************************************************************************
 *
 * File Name:  EpollReactor.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#pragma once
#include <flute/flute-config.h>

#ifdef FLUTE_HAVE_EPOLL
#include <flute/Reactor.h>
#include <vector>

#include <sys/epoll.h>
#ifdef FLUTE_HAVE_SYS_TIMERFD_H
#include <sys/timerfd.h>
#endif

#if defined(FLUTE_HAVE_SYS_TIMERFD_H) && defined(FLUTE_HAVE_TIMERFD_CREATE) && defined(CLOCK_MONOTONIC) && \
    defined(TFD_NONBLOCK) && defined(TFD_CLOEXEC)
#define USING_TIMERFD
#endif

namespace flute {
namespace impl {

class EpollReactor : public Reactor {
public:
    EpollReactor();
    ~EpollReactor();

    void add(socket_type fd, int old, int event, void* data) override;
    void remove(socket_type fd, int old, int event, void* data) override;
    int wait(std::vector<FileEvent>& events, int timeout) override;

private:
    socket_type m_epfd;
#ifdef USING_TIMERFD
    socket_type m_timerfd;
#endif
    std::vector<epoll_event> m_events;

    void open();
    void close();
};

} // namespace impl
} // namespace flute

#endif // FLUTE_HAVE_EPOLL