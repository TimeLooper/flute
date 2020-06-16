//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_TIMER_QUEUE_H
#define FLUTE_TIMER_QUEUE_H

#include <flute/EventLoop.h>
#include <flute/flute-config.h>
#include <flute/noncopyable.h>

#ifdef FLUTE_HAVE_SYS_TIMERFD_H
#include <sys/timerfd.h>
#endif

#if defined(FLUTE_HAVE_SYS_TIMERFD_H) && defined(FLUTE_HAVE_TIMERFD_CREATE) && defined(CLOCK_MONOTONIC) && \
    defined(TFD_NONBLOCK) && defined(TFD_CLOEXEC)
#define USING_TIMERFD
#endif

namespace flute {

class TimerHeap;
struct Timer;

class TimerQueue : private noncopyable {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    std::uint64_t schedule(std::function<void()>&& callback, std::int64_t delay, int loopCount);
    std::uint64_t schedule(const std::function<void()>& callback, std::int64_t delay, int loopCount);
    void cancel(std::uint64_t timerId);
    std::int64_t searchNearestTime(std::int64_t now);
    void handleTimerEvent(std::int64_t now);

private:
    EventLoop* m_loop;
#ifdef USING_TIMERFD
    Channel* m_channel;
#endif
    TimerHeap* m_timerHeap;

    void scheduleInLoop(Timer* timer);
    void cancelTimerInLoop(std::uint64_t timerId);
};

} // namespace flute

#endif // FLUTE_TIMER_QUEUE_H
