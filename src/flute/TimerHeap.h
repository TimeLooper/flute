//
// Created by why on 2019/12/29.
//

#ifndef FLUTE_TIMER_HEAP_H
#define FLUTE_TIMER_HEAP_H

#include <chrono>
#include <cstdint>
#include <functional>
#include <vector>

namespace flute {

inline std::int64_t currentMilliseconds() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
        .count();
}

struct Timer {
    Timer(std::function<void()>&& callback, std::int64_t delay, int loopCount)
        : loopCount(loopCount)
        , index(-1)
        , startTime(currentMilliseconds())
        , delay(delay)
        , callback(std::move(callback)) {}
    Timer(const std::function<void()>& callback, std::int64_t delay, int loopCount)
        : loopCount(loopCount), index(-1), startTime(currentMilliseconds()), delay(delay), callback(callback) {}

    int loopCount;
    int index;
    std::int64_t startTime;
    std::int64_t delay;
    std::function<void()> callback;
};

static inline bool compare(const Timer* lhs, const Timer* rhs) {
    if (lhs->loopCount == 0 && rhs->loopCount != 0) {
        return true;
    }
    if (rhs->loopCount == 0 && lhs->loopCount != 0) {
        return false;
    }
    return lhs->delay + lhs->startTime > rhs->delay + rhs->startTime;
}

class TimerHeap {
public:
    using size_type = typename std::vector<Timer*>::size_type;

    TimerHeap() = default;
    ~TimerHeap() = default;

    Timer* top();
    void pop();
    void remove(Timer* timer);
    void update(Timer* timer);
    void push(Timer* timer);
    bool empty() const;

private:
    std::vector<Timer*> m_timers;

    void shift_down(size_type index);
    void shift_up(size_type index);
};

Timer* TimerHeap::top() { return m_timers.front(); }

void TimerHeap::pop() {
    if (m_timers.empty()) {
        return;
    }
    if (m_timers.size() == 1) {
        m_timers.pop_back();
        return;
    }
    std::swap(m_timers.back(), m_timers.front());
    m_timers.front()->index = 0;
    shift_down(0);
}

void TimerHeap::remove(Timer* timer) {
    if (timer->index < 0 || static_cast<size_type>(timer->index) > m_timers.size()) {
        return;
    }
    auto last = m_timers.back();
    m_timers.pop_back();
    m_timers[timer->index] = last;
    auto parent = (timer->index - 1) / 2;
    if (timer->index > 0 && compare(m_timers[parent], last)) {
        shift_up(timer->index);
    } else {
        shift_down(timer->index);
    }
    timer->index = -1;
}

void TimerHeap::update(Timer* timer) {
    auto parent = (timer->index - 1) / 2;
    if (timer->index > 0 && compare(m_timers[parent], timer)) {
        shift_up(timer->index);
    } else {
        shift_down(timer->index);
    }
}

void TimerHeap::push(Timer* timer) {
    m_timers.push_back(timer);
    shift_up(m_timers.size() - 1);
}

bool TimerHeap::empty() const { return m_timers.empty(); }

void TimerHeap::shift_down(TimerHeap::size_type index) {
    auto temp = m_timers[index];
    auto size = m_timers.size();
    auto min_child = 2 * (index + 1);
    while (min_child <= size) {
        if (min_child == size || compare(m_timers[min_child], m_timers[min_child - 1])) {
            min_child -= 1;
        }
        if (!compare(temp, m_timers[min_child])) {
            break;
        }
        m_timers[index] = m_timers[min_child];
        m_timers[index]->index = static_cast<int>(index);
        index = min_child;
        min_child = 2 * (index + 1);
    }
    m_timers[index] = temp;
    temp->index = static_cast<int>(index);
}

void TimerHeap::shift_up(TimerHeap::size_type index) {
    auto temp = m_timers[index];
    auto parent = (index - 1) / 2;
    while (index && compare(m_timers[parent], temp)) {
        m_timers[index] = m_timers[parent];
        m_timers[index]->index = static_cast<int>(index);
        index = parent;
        parent = (index - 1) / 2;
    }
    m_timers[index] = temp;
    temp->index = static_cast<int>(index);
}

} // namespace flute

#endif // FLUTE_TIMER_HEAP_H
