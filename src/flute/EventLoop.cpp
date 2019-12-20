/*************************************************************************
 *
 * File Name:  EventLoop.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/11/28
 *
 *************************************************************************/

#include <flute/Channel.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopInterrupter.h>
#include <flute/Logger.h>
#include <flute/Reactor.h>
#include <flute/TimerQueue.h>

#include <cerrno>
#include <cstring>

namespace flute {

EventLoop::EventLoop()
    : m_reactor(CHECK_NOTNULL(createReactor()))
    , m_tid(std::this_thread::get_id())
    , m_quit(true)
    , m_isRunTasks(false)
    , m_interrupter(new EventLoopInterrupter(this))
    , m_tasks()
    , m_mutex()
    , m_timerQueue(new TimerQueue(this)) {}

EventLoop::~EventLoop() {
    if (m_interrupter) {
        delete m_interrupter;
        m_interrupter = nullptr;
    }
    if (m_timerQueue) {
        delete m_timerQueue;
        m_timerQueue = nullptr;
    }
    if (m_reactor) {
        delete m_reactor;
        m_reactor = nullptr;
    }
}

void EventLoop::addEvent(Channel* channel, int events) {
    assertInLoopThread();
    m_reactor->add(channel->descriptor(), channel->events(), events, channel);
}

void EventLoop::removeEvent(Channel* channel, int events) {
    assertInLoopThread();
    m_reactor->remove(channel->descriptor(), channel->events(), events, channel);
}

void EventLoop::dispatch() {
    m_quit = false;
    static std::vector<FileEvent> events(32);
    while (!m_quit) {
        auto ret = m_reactor->wait(events, static_cast<int>(m_timerQueue->searchNearestTime()));
        if (ret == -1) {
            LOG_ERROR << "reactor wait " << errno << ": " << std::strerror(errno);
        }
        for (auto i = 0; i < ret; ++i) {
            auto& e = events[i];
            auto ch = static_cast<Channel*>(e.data);
            ch->handleEvent(e.events);
        }
        executeTasks();
        m_timerQueue->handleTimerEvent();
    }
}

void EventLoop::quit() {
    m_quit = true;
    m_interrupter->interrupt();
}

void EventLoop::wakeup() { m_interrupter->interrupt(); }

bool EventLoop::isInLoopThread() const { return m_tid == std::this_thread::get_id(); }

void EventLoop::runInLoop(const std::function<void()>& task) {
    if (isInLoopThread()) {
        task();
    } else {
        queueInLoop(task);
    }
}

void EventLoop::runInLoop(std::function<void()>&& task) {
    if (isInLoopThread()) {
        task();
    } else {
        queueInLoop(std::move(task));
    }
}

std::uint64_t EventLoop::schedule(std::function<void()>&& callback, std::int64_t delay) {
    return m_timerQueue->schedule(std::move(callback), delay, 1);
}

std::uint64_t EventLoop::schedule(std::function<void()>&& callback, std::int64_t delay, int loopCount) {
    return m_timerQueue->schedule(std::move(callback), delay, loopCount);
}

std::uint64_t EventLoop::schedule(const std::function<void()>& callback, std::int64_t delay) {
    return m_timerQueue->schedule(callback, delay, 1);
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
    LOG_FATAL << "EventLoop " << this << " was created in thread " << m_tid << ", current thread id "
              << std::this_thread::get_id() << ".";
}

void EventLoop::queueInLoop(const std::function<void()>& task) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push_back(task);
    }
    if (!isInLoopThread() || m_isRunTasks) {
        wakeup();
    }
}

void EventLoop::queueInLoop(std::function<void()>&& task) {
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_tasks.push_back(std::move(task));
    }
    if (!isInLoopThread() || m_isRunTasks) {
        wakeup();
    }
}

void EventLoop::executeTasks() {
    std::vector<std::function<void()>> temp;
    m_isRunTasks = true;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        temp.swap(m_tasks);
    }
    for (auto& t : temp) {
        t();
    }
    m_isRunTasks = false;
}

} // namespace flute