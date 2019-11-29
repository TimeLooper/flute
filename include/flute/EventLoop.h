/*************************************************************************
 *
 * File Name:  EventLoop.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#pragma once

#include <flute/EventLoopInterrupter.h>
#include <flute/config.h>
#include <flute/noncopyable.h>

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

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
    FLUTE_API_DECL void wakeup();
    FLUTE_API_DECL bool isInLoopThread();
    FLUTE_API_DECL void runInLoop(const std::function<void()>& task);
    FLUTE_API_DECL void runInLoop(std::function<void()>&& task);

private:
    Reactor* m_reactor;
    std::thread::id m_tid;
    std::atomic<bool> m_quit;
    EventLoopInterrupter m_interrupter;
    std::vector<std::function<void()>> m_tasks;

    void queueInLoop(const std::function<void()>& task);
    void queueInLoop(std::function<void()>&& task);
};

} // namespace flute