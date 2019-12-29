//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_REACTOR_H
#define FLUTE_REACTOR_H

#include <flute/EventLoop.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

#include <vector>

namespace flute {

static const int INIT_EVENT_SIZE = 32;
static const int MAX_EVENT_SIZE = 4096;

class Reactor : private noncopyable {
public:
    Reactor() = default;
    virtual ~Reactor() = default;

    virtual void addEvent(socket_type descriptor, int old, int events, void* data) = 0;
    virtual void removeEvent(socket_type descriptor, int old, int events, void* data) = 0;
    virtual int wait(std::vector<FluteEvent>& events, int timeout) = 0;

    static Reactor* createReactor();
};

} // namespace flute

#endif // FLUTE_REACTOR_H
