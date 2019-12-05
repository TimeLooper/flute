/*************************************************************************
 *
 * File Name:  TimerQueue.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/29
 *
 *************************************************************************/

#include <flute/EventLoop.h>
#include <flute/Logger.h>
#include <flute/Timer.h>
#include <flute/TimerHeap.h>
#include <flute/TimerQueue.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <chrono>

namespace flute {

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
    auto timer = reinterpret_cast<Timer*>(timerId);
    m_timerQueue->remove(timer);
    delete timer;
}

} // namespace flute