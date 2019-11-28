/*************************************************************************
 *
 * File Name:  EventLoop.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#pragma once

#include <flute/config.h>
#include <flute/noncopyable.h>
#include <flute/EventLoopInterrupter.h>

namespace flute {
class Reactor;
class Channel;

struct FileEvent {
    static const int NONE = 0x0;
    static const int READ = 0x1 << 1;
    static const int WRITE = 0x1 << 2;

    int events;
    void* data;
};

class EventLoop : private noncopyable {
public:
    FLUTE_API_DECL EventLoop();
    FLUTE_API_DECL ~EventLoop();

    FLUTE_API_DECL void addEvent(Channel* channel, int events);
    FLUTE_API_DECL void removeEvent(Channel* channel, int events);

    FLUTE_API_DECL void dispatch();
    FLUTE_API_DECL void quit();

private:
    Reactor* m_reactor;
    EventLoopInterrupter m_interrupter;
};

} // namespace flute