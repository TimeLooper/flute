//
// Created by why on 2019/12/29.
//

#include <flute/Channel.h>
#include <flute/Logger.h>
#include <flute/TimerHeap.h>
#include <flute/TimerQueue.h>
#include <flute/flute_types.h>
#include <flute/socket_ops.h>

#include <cassert>
#include <cstring>

namespace flute {

TimerQueue::TimerQueue(flute::EventLoop* loop)
    : m_loop(loop)
#ifdef USING_TIMERFD
    , m_channel(nullptr)
#endif
    , m_timerHeap(new TimerHeap()) {
#ifdef USING_TIMERFD
    auto descriptor = ::timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC | TFD_NONBLOCK);
    if (descriptor != FLUTE_INVALID_SOCKET) {
        m_channel = new Channel(descriptor, loop);
        m_channel->enableRead();
    }
#endif
}

TimerQueue::~TimerQueue() {
#ifdef USING_TIMERFD
    if (m_channel) {
        m_channel->disableAll();
        flute::close(m_channel->descriptor());
        delete m_channel;
    }
#endif
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
    if (m_channel) {
        itimerspec spec{};
        spec.it_value.tv_nsec = (delay % 1000) * 1000000;
        spec.it_value.tv_sec = delay / 1000;
        auto ret = ::timerfd_settime(m_channel->descriptor(), 0, &spec, nullptr);
        if (ret == 0) {
            delay = -1;
        } else {
            auto error = getLastError();
            LOG_ERROR << "timerfd_settime error " << error << ":" << formatErrorString(error);
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
    std::vector<Timer *> timers;
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
            timers.push_back(timer);
            m_timerHeap->remove(timer);
        } else {
            break;
        }
    }
    currentTime = currentMilliseconds();
    for (auto timer : timers) {
        if (timer->loopCount > 0 || timer->loopCount == -1) {
            timer->startTime = currentTime;
            m_timerHeap->push(timer);
        } else {
            delete timer;
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
