/*************************************************************************
 *
 * File Name:  KqueueReactor.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/29 00:29:12
 *
 *************************************************************************/

#pragma once
#include <flute/flute-config.h>
#ifdef FLUTE_HAVE_KQUEUE
#include <flute/config.h>
#include <flute/Reactor.h>
#include <sys/event.h>

namespace flute {
namespace impl {

class KqueueReactor : public Reactor {
public:
    KqueueReactor();
    ~KqueueReactor();

    void add(socket_type fd, int old, int event, void* data) override;
    void remove(socket_type fd, int old, int event, void* data) override;
    int wait(std::vector<FileEvent>& events, int timeout) override;

private:
    socket_type m_kqfd;
    std::vector<struct kevent> m_events;

    void open();
    void close();
};

} // namespace impl
} // namespace flute

#endif