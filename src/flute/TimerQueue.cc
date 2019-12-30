//
// Created by why on 2019/12/29.
//

#include <flute/TimerHeap.h>
#include <flute/TimerQueue.h>
#include <flute/flute_types.h>

#include <cassert>

namespace flute {

TimerQueue::TimerQueue(flute::EventLoop* loop)
    : m_timerDescriptor(FLUTE_INVALID_SOCKET), m_loop(loop), m_timerHeap(new TimerHeap()) {
#ifdef USING_TIMERFD
    m_timerDescriptor = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
#endif
}

TimerQueue::~TimerQueue() {
    assert(m_timerHeap->empty());
    delete m_timerHeap;
}

std::uint64_t TimerQueue::schedule(std::function<void()>&& callback, std::int64_t delay, int loopCount) {
    auto timer = new Timer(std::move(callback), delay, loopCount);
    m_loop->runInLoop(std::bind(&TimerQueue::scheduleInLoop, this, timer));
    return reinterpret_cast<std::uint64_t>(timer);
}

std::uint64_t TimerQueue::schedule(const std::function<void()>& callback, std::int64_t delay, int loopCount) {
    auto timer = new Timer(callback, delay, loopCount);
    m_loop->runInLoop(std::bind(&TimerQueue::scheduleInLoop, this, timer));
    return reinterpret_cast<std::uint64_t>(timer);
}

void TimerQueue::cancel(std::uint64_t timerId) {
    m_loop->runInLoop(std::bind(&TimerQueue::cancelTimerInLoop, this, timerId));
}

std::int64_t TimerQueue::searchNearestTime() {
    if (m_timerHeap->empty()) {
        return -1;
    }
    auto top = m_timerHeap->top();
    auto delay = top->delay - currentMilliseconds() + top->startTime;
#ifdef USING_TIMERFD
    if (m_timerDescriptor != FLUTE_INVALID_SOCKET) {
        itimerspec spec{};
        spec.it_value.tv_nsec = (delay % 1000) * 1000;
        spec.it_value.tv_sec = delay / 1000;
        auto ret = ::timerfd_settime(m_timerDescriptor, 0, &spec, nullptr);
        if (ret == 0) {
            delay = -1;
        }
    }
#endif
    return delay;
}

void TimerQueue::handleTimerEvent() {
    if (m_timerHeap->empty()) {
        return;
    }
    auto currentTime = currentMilliseconds();
    while (!m_timerHeap->empty()) {
        auto timer = m_timerHeap->top();
        auto offset = timer->delay + timer->startTime - currentTime;
        if (offset <= 0) {
            if (timer->callback) {
                timer->callback();
            }
            if (timer->loopCount > 0) {
                timer->loopCount -= 1;
            }
            if (timer->loopCount == 0) {
                m_timerHeap->remove(timer);
                delete timer;
            } else {
                timer->startTime += timer->delay;
                m_timerHeap->update(timer);
            }
        } else {
            break;
        }
    }
}

void TimerQueue::scheduleInLoop(Timer* timer) { m_timerHeap->push(timer); }

void TimerQueue::cancelTimerInLoop(std::uint64_t timerId) {
    auto timer = reinterpret_cast<Timer*>(timerId);
    m_timerHeap->remove(timer);
    delete timer;
}

} // namespace flute
