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
#include <flute/Reactor.h>

namespace flute {

EventLoop::EventLoop() : m_reactor(createReactor()), m_tid(), m_quit(true), m_interrupter(this), m_tasks() {
}

EventLoop::~EventLoop() {
}

void EventLoop::addEvent(Channel* channel, int events) {
    m_reactor->add(channel->descriptor(), channel->events(), events, channel);
}

void EventLoop::removeEvent(Channel* channel, int events) {
    m_reactor->remove(channel->descriptor(), channel->events(), events, channel);
}

void EventLoop::dispatch() {
    m_quit = false;
    m_tid = std::this_thread::get_id();
    static std::vector<FileEvent> events;
    while (!m_quit) {
        auto ret = m_reactor->wait(events, -1);
    }
}

void EventLoop::quit() {
    m_quit = true;
    m_interrupter.interrupt();
}

void EventLoop::wakeup() {
    m_interrupter.interrupt();
}

bool EventLoop::isInLoopThread() {
    m_tid == std::this_thread::get_id();
}

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

void EventLoop::queueInLoop(const std::function<void()>& task) {
}

void EventLoop::queueInLoop(std::function<void()>&& task) {
}

} // namespace flute