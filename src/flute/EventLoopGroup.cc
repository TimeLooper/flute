//
// Created by why on 2019/12/30.
//

#include <flute/AsyncIoService.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopGroup.h>
#include <flute/Logger.h>

#include <memory>

namespace flute {

EventLoopGroup::EventLoopGroup(const EventLoopGroupConfigure& configure)
    : m_masterEventLoop(nullptr)
    , m_asyncIoService(nullptr)
    , m_slaveEventLoops()
    , m_threadPool()
    , m_main_thread()
    , m_close_promise() {
    m_threadPool.start(configure.childLoopSize);
    m_slaveEventLoops.reserve(configure.childLoopSize);
    for (std::size_t i = 0; i < configure.childLoopSize; ++i) {
        std::promise<EventLoop*> p;
        m_threadPool.execute([&p] {
            EventLoop loop;
            p.set_value(&loop);
            loop.dispatch();
        });
        auto loop = p.get_future().get();
        m_slaveEventLoops.emplace_back(loop);
    }
    std::promise<EventLoop*> p;
    m_main_thread = std::thread([this, &p] {
        auto loop = new EventLoop();
        p.set_value(loop);
        loop->dispatch();
    });
    m_masterEventLoop = p.get_future().get();
    if (configure.useAsyncIo) {
        m_asyncIoService = AsyncIoService::createAsyncIoService(configure.asyncIoWorkThreadCount);
        m_masterEventLoop->setAsyncIoService(m_asyncIoService);
        for (auto& loop : m_slaveEventLoops) {
            loop->setAsyncIoService(m_asyncIoService);
        }
    }
}

EventLoopGroup::~EventLoopGroup() {
    if (m_masterEventLoop) {
        delete m_masterEventLoop;
        m_masterEventLoop = nullptr;
    }
    if (m_asyncIoService) {
        delete m_asyncIoService;
        m_asyncIoService = nullptr;
    }
}

EventLoop* EventLoopGroup::chooseSlaveEventLoop(std::uint64_t hash) {
    auto size = static_cast<std::int64_t>(m_slaveEventLoops.size());
    if (size <= 0) {
        return m_masterEventLoop;
    }
    if ((size & -size) == size) {
        return m_slaveEventLoops[hash & (size - 1)];
    } else {
        return m_slaveEventLoops[hash % size];
    }
}

EventLoop* EventLoopGroup::getMasterEventLoop() { return m_masterEventLoop; }

void EventLoopGroup::wait() {
    m_close_promise.get_future().get();
    m_threadPool.shutdown();
    if (m_asyncIoService) {
        m_asyncIoService->shutdown();
    }
    if (m_main_thread.joinable()) {
        m_main_thread.join();
    }
}

void EventLoopGroup::shutdown() {
    for (auto& loop : m_slaveEventLoops) {
        loop->quit();
    }
    m_masterEventLoop->quit();
    m_close_promise.set_value();
}

} // namespace flute