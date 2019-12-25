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
#include <flute/Logger.h>

#include <memory>

namespace flute {

EventLoopGroup::EventLoopGroup(std::size_t size) : m_index(0), m_eventLoops(), m_threadPool() {
    m_threadPool.start(size);
    m_eventLoops.reserve(size);
    for (std::size_t i = 0; i < size; ++i) {
        std::promise<EventLoop*> p;
        m_threadPool.execute([&p] {
            EventLoop loop;
            p.set_value(&loop);
            loop.dispatch();
            LOG_FATAL << "exit loop";
        });
        auto loop = p.get_future().get();
        m_eventLoops.emplace_back(loop);
    }
}

EventLoopGroup::~EventLoopGroup() {}

EventLoop* EventLoopGroup::chooseEventLoop(std::uint64_t hash) {
    auto size = m_eventLoops.size();
    if ((size & -size) == size) {
        return m_eventLoops[hash & (size - 1)];
    } else {
        return m_eventLoops[hash % size];
    }
}

EventLoop* EventLoopGroup::chooseEventLoop() {
    auto result = m_eventLoops[m_index];
    m_index += 1;
    std::size_t idx = m_index;
    m_index = idx >= m_eventLoops.size() ? 0 : idx;
    return result;
}

void EventLoopGroup::shutdown() {
    for (auto& loop : m_eventLoops) {
        loop->quit();
    }
    m_threadPool.shutdown();
}

} // namespace flute