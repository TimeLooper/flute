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
#include <flute/Timer.h>
#include <flute/TimerHeap.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>

namespace flute {

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
        auto it = search(value);
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

private:
    inline std::vector<Timer *>::iterator search(const Timer* value) {
        return std::find(c.begin(), c.end(), value);
    }
};

TimerQueue::TimerQueue(EventLoop* loop) : m_loop(loop), m_timerQueue(new TimerHeap()) {
}

TimerQueue::~TimerQueue() {
    assert(m_timerQueue->empty());
    delete m_timerQueue;
}

std::uint64_t TimerQueue::schedule(std::function<void()>&& callback, std::int64_t delay, int loopCount) {
    auto timer = new Timer(std::move(callback), delay, loopCount);
    m_loop->runInLoop(std::bind(&TimerQueue::postTimerInLoop, this, timer));
    return reinterpret_cast<std::uint64_t>(timer);
}

std::uint64_t TimerQueue::schedule(const std::function<void()>& callback, std::int64_t delay, int loopCount) {
    auto timer = new Timer(callback, delay, loopCount);
    m_loop->runInLoop(std::bind(&TimerQueue::postTimerInLoop, this, timer));
    return reinterpret_cast<std::uint64_t>(timer);
}

void TimerQueue::cancel(std::uint64_t timerId) {
    m_loop->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop, this, timerId));
}

std::int64_t TimerQueue::searchNearestTime() {
    if (m_timerQueue->empty()) {
        return -1;
    }
    auto top = m_timerQueue->top();
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
                m_timerQueue->remove(timer);
                delete timer;
            } else {
                timer->startTime += timer->delay;
                m_timerQueue->update(timer);
            }
        } else {
            break;
        }
    }
}

void TimerQueue::postTimerInLoop(Timer* timer) {
    m_timerQueue->push(timer);
}

void TimerQueue::cancelTimerInLoop(std::uint64_t timerId) {
    auto timer = reinterpret_cast<Timer *>(timerId);
    m_timerQueue->remove(timer);
    delete timer;
}

} // namespace flute