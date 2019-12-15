/*************************************************************************
 *
 * File Name:  EventLoopGroup.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/15
 *
 *************************************************************************/

#include <flute/EventLoop.h>
#include <flute/EventLoopGroup.h>

#include <memory>

namespace flute {

EventLoopGroup::EventLoopGroup(std::size_t size) : m_eventLoops(), m_threadPool() {
    m_threadPool.start(size);
    m_eventLoops.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        auto loop = new EventLoop();
        m_eventLoops[i].reset(loop);
        m_threadPool.execute(&EventLoopGroup::dispatch, this, loop);
    }
}

EventLoopGroup::~EventLoopGroup() {
}

EventLoop* EventLoopGroup::chooseEventLoop(std::uint64_t hash) {
    return nullptr;
}

void EventLoopGroup::shutdown() {
    for (auto& loop : m_eventLoops) {
        loop->quit();
    }
    m_threadPool.shutdown();
}

void EventLoopGroup::dispatch(EventLoop* loop) {
    loop->attachThread();
    loop->dispatch();
}

} // namespace flute