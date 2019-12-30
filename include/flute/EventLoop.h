//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_EVENTLOOP_H
#define FLUTE_EVENTLOOP_H

#include <flute/config.h>
#include <flute/noncopyable.h>

#include <atomic>
#include <functional>
#include <mutex>
#include <thread>
#include <vector>

namespace flute {

struct FluteEvent {
    static const int NONE = 0x0;
    static const int READ = 0x1;
    static const int WRITE = 0x2;

    int events;
    void* data;
};

class Selector;
class TimerQueue;
class EventLoopInterruptor;
class Channel;

class EventLoop : private noncopyable {
public:
    FLUTE_API_DECL EventLoop();
    FLUTE_API_DECL ~EventLoop();

    inline bool isInLoopThread() const { return m_tid == std::this_thread::get_id(); }
    FLUTE_API_DECL void addEvent(Channel* channel, int events);
    FLUTE_API_DECL void removeEvent(Channel* channel, int events);
    FLUTE_API_DECL void dispatch();
    FLUTE_API_DECL void quit();
    FLUTE_API_DECL void interrupt();
    FLUTE_API_DECL void runInLoop(std::function<void()>&& task);
    FLUTE_API_DECL void runInLoop(const std::function<void()>& task);
    FLUTE_API_DECL void queueInLoop(std::function<void()>&& task);
    FLUTE_API_DECL void queueInLoop(const std::function<void()>& task);
    FLUTE_API_DECL std::uint64_t schedule(std::function<void()>&& callback, std::int64_t delay, int loopCount);
    FLUTE_API_DECL std::uint64_t schedule(const std::function<void()>& callback, std::int64_t delay, int loopCount);
    FLUTE_API_DECL void cancel(std::uint64_t timerId);
    FLUTE_API_DECL void assertInLoopThread() const;
    FLUTE_API_DECL void abortNotInLoopThread() const;

private:
    Selector* m_selector;
    const std::thread::id m_tid;
    EventLoopInterruptor* m_interruptor;
    TimerQueue* m_timerQueue;
    std::atomic<bool> m_quit;
    std::atomic<bool> m_isRunTasks;
    std::vector<std::function<void()>> m_tasks;
    std::mutex m_mutex;

    void executeTasks();
};

} // namespace flute

#endif // FLUTE_EVENTLOOP_H
