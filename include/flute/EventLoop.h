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

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace flute {

class Reactor;
class Channel;
class EventLoopInterrupter;
class TimerQueue;

struct FileEvent {
    static const int NONE = 0x0;
    static const int READ = 0x1 << 0;
    static const int WRITE = 0x1 << 1;

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
    FLUTE_API_DECL bool isInLoopThread() const;
    FLUTE_API_DECL void runInLoop(const std::function<void()>& task);
    FLUTE_API_DECL void runInLoop(std::function<void()>&& task);
    FLUTE_API_DECL void queueInLoop(const std::function<void()>& task);
    FLUTE_API_DECL void queueInLoop(std::function<void()>&& task);
    FLUTE_API_DECL std::uint64_t schedule(std::function<void()>&& callback, std::int64_t delay);
    FLUTE_API_DECL std::uint64_t schedule(std::function<void()>&& callback, std::int64_t delay, int loopCount);
    FLUTE_API_DECL std::uint64_t schedule(const std::function<void()>& callback, std::int64_t delay);
    FLUTE_API_DECL std::uint64_t schedule(const std::function<void()>& callback, std::int64_t delay, int loopCount);
    FLUTE_API_DECL void cancel(std::uint64_t timerId);
    FLUTE_API_DECL void assertInLoopThread() const;
    FLUTE_API_DECL void abortNotInLoopThread() const;

private:
    Reactor* m_reactor;
    const std::thread::id m_tid;
    std::atomic<bool> m_quit;
    std::atomic<bool> m_isRunTasks;
    EventLoopInterrupter* m_interrupter;
    std::vector<std::function<void()>> m_tasks;
    std::mutex m_mutex;
    TimerQueue* m_timerQueue;

    void executeTasks();
};

} // namespace flute