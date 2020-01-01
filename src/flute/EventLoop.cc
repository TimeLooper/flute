//
// Created by why on 2019/12/29.
//

#include <flute/Channel.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopInterruptor.h>
#include <flute/Logger.h>
#include <flute/Selector.h>
#include <flute/TimerQueue.h>

namespace flute {

EventLoop::EventLoop()
    : m_selector(Selector::createSelector())
    , m_tid(std::this_thread::get_id())
    , m_interruptor(new EventLoopInterruptor(this))
    , m_timerQueue(new TimerQueue(this))
    , m_quit(true)
    , m_isRunTasks(false)
    , m_tasks()
    , m_mutex() {}

EventLoop::~EventLoop() {
    if (m_timerQueue) {
        delete m_timerQueue;
        m_timerQueue = nullptr;
    }
    if (m_interruptor) {
        delete m_interruptor;
        m_interruptor = nullptr;
    }
    if (m_selector) {
        delete m_selector;
        m_selector = nullptr;
    }
}

void EventLoop::addEvent(Channel* channel, int events) {
    m_selector->addEvent(channel->descriptor(), channel->events(), events, channel);
}

void EventLoop::removeEvent(Channel* channel, int events) {
    m_selector->removeEvent(channel->descriptor(), channel->events(), events, channel);
}

void EventLoop::dispatch() {
    m_quit = false;
    std::vector<SelectorEvent> events(32);
    while (!m_quit) {
        auto ret = m_selector->select(events, static_cast<int>(m_timerQueue->searchNearestTime()));
        if (ret == -1) {
            continue;
        }
        m_timerQueue->handleTimerEvent();
        for (auto i = 0; i < ret; ++i) {
            auto& e = events[i];
            auto ch = static_cast<Channel*>(e.data);
            ch->handleEvent(e.events);
        }
        executeTasks();
    }
}

void EventLoop::quit() {
    m_quit = true;
    m_interruptor->interrupt();
}

void EventLoop::interrupt() { m_interruptor->interrupt(); }

void EventLoop::runInLoop(std::function<void()>&& task) {
    if (isInLoopThread()) {
        task();
    } else {
        queueInLoop(std::move(task));
    }
}

void EventLoop::runInLoop(const std::function<void()>& task) {
    if (isInLoopThread()) {
        task();
    } else {
        queueInLoop(task);
    }
}

void EventLoop::queueInLoop(std::function<void()>&& task) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push_back(std::move(task));
    }
    if (!isInLoopThread() || m_isRunTasks) {
        m_interruptor->interrupt();
    }
}

void EventLoop::queueInLoop(const std::function<void()>& task) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push_back(task);
    }
    if (!isInLoopThread() || m_isRunTasks) {
        m_interruptor->interrupt();
    }
}

std::uint64_t EventLoop::schedule(std::function<void()>&& callback, std::int64_t delay, int loopCount) {
    return m_timerQueue->schedule(std::move(callback), delay, loopCount);
}

std::uint64_t EventLoop::schedule(const std::function<void()>& callback, std::int64_t delay, int loopCount) {
    return m_timerQueue->schedule(callback, delay, loopCount);
}

void EventLoop::cancel(std::uint64_t timerId) { m_timerQueue->cancel(timerId); }

void EventLoop::assertInLoopThread() const {
    if (!isInLoopThread()) {
        abortNotInLoopThread();
    }
}

void EventLoop::abortNotInLoopThread() const {
    LOG_FATAL << "EventLoop " << this << " was create in thread " << m_tid << ", current thread id "
              << std::this_thread::get_id() << ".";
}

void EventLoop::executeTasks() {
    std::vector<std::function<void()>> temp;
    m_isRunTasks = true;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        temp.swap(m_tasks);
    }
    for (const auto& t : temp) {
        t();
    }
    m_isRunTasks = false;
}

} // namespace flute