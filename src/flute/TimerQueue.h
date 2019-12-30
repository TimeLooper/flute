/*************************************************************************
 *
 * File Name:  TimerQueue.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/29
 *
 *************************************************************************/

#pragma once
#include <flute/flute-config.h>
#include <flute/flute_types.h>
#include <flute/noncopyable.h>

#include <cstdint>
#include <functional>
#include <map>
#include <queue>

#ifdef FLUTE_HAVE_SYS_TIMERFD_H
#include <sys/timerfd.h>
#endif

#if defined(FLUTE_HAVE_SYS_TIMERFD_H) && defined(FLUTE_HAVE_TIMERFD_CREATE) && defined(CLOCK_MONOTONIC) && \
    defined(TFD_NONBLOCK) && defined(TFD_CLOEXEC)
#define USING_TIMERFD
#endif

namespace flute {

class Timer;
class EventLoop;
class TimerHeap;

class TimerQueue : private noncopyable {
public:
    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    std::uint64_t schedule(std::function<void()>&& callback, std::int64_t delay, int loopCount);
    std::uint64_t schedule(const std::function<void()>& callback, std::int64_t delay, int loopCount);
    void cancel(std::uint64_t timerId);
    std::int64_t searchNearestTime();

private:
    EventLoop* m_loop;
#ifdef USING_TIMERFD
    Channel* m_channel;
#endif
    TimerHeap* m_timerHeap;

    void postTimerInLoop(Timer* timer);
    void cancelTimerInLoop(std::uint64_t timerId);
    void handleTimerEvent();
    friend class EventLoop;
};

} // namespace flute