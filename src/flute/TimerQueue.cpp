/*************************************************************************
 *
 * File Name:  TimerQueue.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/29
 *
 *************************************************************************/

#include <flute/EventLoop.h>
#include <flute/TimerQueue.h>

#include <atomic>
#include <cassert>
#include <chrono>
#include <algorithm>

namespace flute {

static inline std::int64_t currentMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

class Timer {
public:
    Timer(std::function<void()>&& callback, std::int64_t delay, int loopCount)
        : loopCount(loopCount)
        , id(++s_numCreated)
        , startTime(currentMilliseconds())
        , delay(delay)
        , callback(std::move(callback)) {
    }
    Timer(const std::function<void()>& callback, std::int64_t delay, int loopCount)
        : loopCount(loopCount), id(++s_numCreated), startTime(currentMilliseconds()), delay(delay), callback(callback) {
    }

    int loopCount;
    std::uint64_t id;
    std::int64_t startTime;
    std::int64_t delay;
    std::function<void()> callback;

    static std::atomic<std::uint64_t> s_numCreated;
};

struct TimerCompare : public std::greater<Timer*> {
    bool operator()(const Timer* lhs, const Timer* rhs) {
        return lhs->delay + lhs->startTime > rhs->delay + rhs->startTime;
    }
} timerCompare;

std::atomic<std::uint64_t> Timer::s_numCreated;

TimerQueue::TimerQueue(EventLoop* loop) : m_loop(loop), m_timerQueue(), m_timerMap() {
}

TimerQueue::~TimerQueue() {
    assert(m_timerQueue.empty());
    assert(m_timerMap.empty());
}

std::uint64_t TimerQueue::schedule(std::function<void()>&& callback, std::int64_t delay, int loopCount) {
    auto timer = new Timer(std::move(callback), delay, loopCount);
    m_loop->runInLoop(std::bind(&TimerQueue::postTimerInLoop, this, timer));
    return timer->id;
}

std::uint64_t TimerQueue::schedule(const std::function<void()>& callback, std::int64_t delay, int loopCount) {
    auto timer = new Timer(callback, delay, loopCount);
    m_loop->runInLoop(std::bind(&TimerQueue::postTimerInLoop, this, timer));
    return timer->id;
}

void TimerQueue::cancel(std::uint64_t timerId) {
    m_loop->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop, this, timerId));
}

std::int64_t TimerQueue::searchNearestTime() {
    if (m_timerQueue.empty()) {
        return -1;
    }
    auto& top = m_timerQueue.front();
    auto delay = top->delay - currentMilliseconds() + top->startTime;
    return delay;
}

void TimerQueue::handleTimerEvent() {
    auto currentTime = currentMilliseconds();
    std::vector<Timer*> temp;
    for (auto it = m_timerQueue.begin(); it != m_timerQueue.end(); ++it) {
        auto& top = *it;
        auto offset = top->delay + top->startTime - currentTime;
        if (offset <= 0) {
            if (top->callback) {
                top->callback();
            }
            if (top->loopCount > 0) {
                top->loopCount -= 1;
            }
            if (top->loopCount == 0) {
                m_timerMap.erase((*it)->id);
                delete (*it);
            } else {
                temp.push_back(*it);
            }
        } else {
            break;
        }
        std::make_heap(it, m_timerQueue.end(), timerCompare);
    }
    m_timerQueue.swap(temp);
    std::make_heap(m_timerQueue.begin(), m_timerQueue.end(), timerCompare);
}

void TimerQueue::postTimerInLoop(Timer* timer) {
    auto it = m_timerMap.find(timer->id);
    assert(it == m_timerMap.end());
    m_timerQueue.push_back(timer);
    std::push_heap(m_timerQueue.begin(), m_timerQueue.end(), timerCompare);
    m_timerMap[timer->id] = timer;
}

void TimerQueue::cancelTimerInLoop(std::uint64_t timerId) {
    auto it = m_timerMap.find(timerId);
    if (it == m_timerMap.end()) {
        return;
    }
    m_timerMap.erase(it);
    auto temp =
        std::find_if(m_timerQueue.begin(), m_timerQueue.end(), [&](const Timer* t) { return t->id == timerId; });
    if (temp != m_timerQueue.end()) {
        m_timerQueue.erase(temp);
        std::make_heap(m_timerQueue.begin(), m_timerQueue.end(), timerCompare);
    }
    delete it->second;
    m_loop->wakeup();
}

} // namespace flute