//
// Created by why on 2019/12/30.
//

#ifndef FLUTE_EVENT_LOOP_GROUP_H
#define FLUTE_EVENT_LOOP_GROUP_H

#include <flute/ThreadPool.h>
#include <flute/config.h>
#include <flute/noncopyable.h>

#include <memory>

namespace flute {

class EventLoopGroup : private noncopyable {
public:
    FLUTE_API_DECL explicit EventLoopGroup(std::size_t childSize);
    FLUTE_API_DECL ~EventLoopGroup();

    FLUTE_API_DECL void shutdown();
    FLUTE_API_DECL EventLoop* chooseSlaveEventLoop(std::uint64_t hash);
    FLUTE_API_DECL EventLoop* getMasterEventLoop();
    FLUTE_API_DECL void dispatch();

private:
    EventLoop* m_masterEventLoop;
    std::vector<EventLoop *> m_slaveEventLoops;
    ThreadPool m_threadPool;
};

} // namespace flute

#endif // FLUTE_EVENT_LOOP_GROUP_H