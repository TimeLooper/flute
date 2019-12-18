/*************************************************************************
 *
 * File Name:  EventLoopGroup.h
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/15
 *
 *************************************************************************/

#pragma once

#include <flute/ThreadPool.h>
#include <flute/config.h>

#include <cstdint>

namespace flute {

class EventLoop;

class EventLoopGroup {
public:
    FLUTE_API_DECL explicit EventLoopGroup(std::size_t);
    FLUTE_API_DECL ~EventLoopGroup();

    FLUTE_API_DECL void shutdown();
    FLUTE_API_DECL EventLoop* chooseEventLoop(std::uint64_t hash);

private:
    std::vector<std::unique_ptr<EventLoop>> m_eventLoops;
    ThreadPool m_threadPool;
};

} // namespace flute
