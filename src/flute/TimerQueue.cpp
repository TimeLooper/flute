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
#include <flute/Logger.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>

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
        if (lhs->loopCount == 0 && rhs->loopCount != 0) {
            return true;
        }
        if (rhs->loopCount ==  0 && lhs->loopCount != 0) {
            return false;
        }
        return lhs->delay + lhs->startTime > rhs->delay + rhs->startTime;
    }
};

class TimerQueue::timer_queue : public std::priority_queue<Timer *, std::vector<Timer *>, TimerCompare> {
public:
    bool remove(const Timer* value) {
        auto it = std::find(this->c.begin(), this->c.end(), value);
        if (it != this->c.end()) {
            this->c.erase(it);
            std::make_heap(this->c.begin(), this->c.end(), this->comp);
            return true;
        } else {
            return false;
        }
    }
    void rebuild_heap() {
        std::make_heap(this->c.begin(), this->c.end(), this->comp);
    }
};

std::atomic<std::uint64_t> Timer::s_numCreated;

TimerQueue::TimerQueue(EventLoop* loop) : m_loop(loop), m_timerQueue(new timer_queue()), m_timerMap() {
}

TimerQueue::~TimerQueue() {
    assert(m_timerQueue->empty());
    assert(m_timerMap.empty());
    delete m_timerQueue;
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
    if (m_timerQueue->empty()) {
        return -1;
    }
    auto& top = m_timerQueue->top();
    auto delay = top->delay - currentMilliseconds() + top->startTime;
    return delay;
}

void TimerQueue::handleTimerEvent() {
    if (m_timerQueue->empty()) {
        return;
    }
    auto currentTime = currentMilliseconds();
    while (!m_timerQueue->empty()) {
        auto timer = m_timerQueue->top();
        auto offset = timer->delay + timer->startTime - currentTime;
        if (offset <= 0) {
            if (timer->callback) {
                timer->callback();
            }
            if (timer->loopCount > 0) {
                timer->loopCount -= 1;
            }
            if (timer->loopCount == 0) {
                m_timerMap.erase(timer->id);
                m_timerQueue->remove(timer);
                delete timer;
            } else {
                timer->startTime += timer->delay;
                m_timerQueue->rebuild_heap();
            }
        } else {
            break;
        }
    }
}

void TimerQueue::postTimerInLoop(Timer* timer) {
    auto it = m_timerMap.find(timer->id);
    (void)it;
    assert(it == m_timerMap.end());
    m_timerQueue->push(timer);
    m_timerMap[timer->id] = timer;
}

void TimerQueue::cancelTimerInLoop(std::uint64_t timerId) {
    auto it = m_timerMap.find(timerId);
    if (it == m_timerMap.end()) {
        return;
    }
    m_timerMap.erase(it);
    m_timerQueue->remove(it->second);
    delete it->second;
}

} // namespace flute