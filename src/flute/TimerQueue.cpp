/*************************************************************************
 *
 * File Name:  TimerQueue.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/29
 *
 *************************************************************************/

#include <flute/TimerQueue.h>
#include <flute/EventLoop.h>

#include <cassert>
#include <atomic>

namespace flute {

class Timer {
public:
    Timer(std::function<void()>&& callback, std::int64_t delay, int loopCount)
        : loopCount(loopCount), delay(delay), id(++s_numCreated), callback(std::move(callback)) {
    }
    Timer(const std::function<void()>& callback, std::int64_t delay, int loopCount)
        : loopCount(loopCount), delay(delay), id(++s_numCreated), callback(callback) {
    }

    int loopCount;
    std::int64_t delay;
    std::uint64_t id;
    std::function<void()> callback;

    static std::atomic<std::uint64_t> s_numCreated;
};

std::atomic<std::uint64_t> Timer::s_numCreated;

TimerQueue::TimerQueue(EventLoop* loop) : m_loop(loop), m_timerQueue(), m_timerMap() {
}

TimerQueue::~TimerQueue() {
}

std::uint64_t TimerQueue::postTimer(std::function<void()>&& callback, std::int64_t delay, int loopCount) {
    auto timer = new Timer(std::move(callback), delay, loopCount);
    m_loop->runInLoop(std::bind(&TimerQueue::postTimerInLoop, this, timer));
    return timer->id;
}

std::uint64_t TimerQueue::postTimer(const std::function<void()>& callback, std::int64_t delay, int loopCount) {
    auto timer = new Timer(callback, delay, loopCount);
    m_loop->runInLoop(std::bind(&TimerQueue::postTimerInLoop, this, timer));
    return timer->id;
}

void TimerQueue::cancel(std::uint64_t timerId) {
    m_loop->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop, this, timerId));
}

void TimerQueue::postTimerInLoop(Timer* timer) {
    auto it = m_timerMap.find(timer->id);
    assert(it == m_timerMap.end());
    m_timerQueue.push(timer);
    m_timerMap[timer->id] = timer;
}

void TimerQueue::cancelTimerInLoop(std::uint64_t timerId) {
    auto it = m_timerMap.find(timerId);
    if (it == m_timerMap.end()) {
        return;
    }
    m_timerMap.erase(it);
    std::vector<Timer *> temp;
    delete it->second;
}

} // namespace flute