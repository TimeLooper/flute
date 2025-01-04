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

class EventLoop;
class AsyncIoService;

struct EventLoopGroupConfigure {
    bool useAsyncIo;
    std::size_t childLoopSize;
    std::size_t asyncIoWorkThreadCount;
    EventLoopGroupConfigure() : useAsyncIo(false), childLoopSize(0), asyncIoWorkThreadCount(0) {}
    EventLoopGroupConfigure(bool useAsyncIo, std::size_t childLoopSize, std::size_t asyncIoWorkThreadCount)
        : useAsyncIo(useAsyncIo), childLoopSize(childLoopSize), asyncIoWorkThreadCount(asyncIoWorkThreadCount) {}
};

class EventLoopGroup : private noncopyable {
public:
    FLUTE_API_DECL explicit EventLoopGroup(const EventLoopGroupConfigure& configure);
    FLUTE_API_DECL ~EventLoopGroup();

    FLUTE_API_DECL void shutdown();
    FLUTE_API_DECL EventLoop* chooseSlaveEventLoop(std::uint64_t hash);
    FLUTE_API_DECL EventLoop* getMasterEventLoop();
    FLUTE_API_DECL void dispatch();

    inline std::size_t getChildLoopSize() const { return m_configure.childLoopSize; }

private:
    EventLoop* m_masterEventLoop;
    AsyncIoService* m_asyncIoService;
    EventLoopGroupConfigure m_configure;
    std::vector<EventLoop*> m_slaveEventLoops;
    ThreadPool m_threadPool;
};

} // namespace flute

#endif // FLUTE_EVENT_LOOP_GROUP_H