//
// Created by why on 2019/12/30.
//

#include <flute/Acceptor.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/TcpConnection.h>
#include <flute/TcpServer.h>

#include <cassert>

namespace flute {

TcpServer::TcpServer(flute::EventLoopGroup* eventLoopGroup)
    : m_eventLoopGroup(eventLoopGroup)
    , m_state(ServerState::STOPPED)
    , m_acceptor()
    , m_connections()
    , m_messageCallback()
    , m_writeCompleteCallback()
    , m_highWaterMarkCallback()
    , m_connectionDestroyCallback()
    , m_closeCallback()
    , m_close_promise() {}

TcpServer::~TcpServer() {
    for (auto& it : m_connections) {
        auto conn = it.second;
        conn->handleConnectionDestroy();
    }
}

void TcpServer::bind(const InetAddress& address) {
    if (m_state != ServerState::STOPPED) {
        return;
    }
    m_state = ServerState::STARTED;
    m_acceptor.reset(new Acceptor(m_eventLoopGroup->getMasterEventLoop()));
    m_acceptor->setReusePort(true);
    m_acceptor->setReuseAddress(true);
    m_acceptor->bind(address);
    m_acceptor->setAcceptCallback(std::bind(&TcpServer::handleAcceptConnection, this, std::placeholders::_1));
    assert(!m_acceptor->listening());
    m_acceptor->listen();
}

void TcpServer::close() {
    assert(m_state != ServerState::STOPPING);
    if (m_state == ServerState::STOPPING) {
        return;
    }
    m_eventLoopGroup->getMasterEventLoop()->queueInLoop(std::bind(&TcpServer::closeInLoop, this));
    m_close_promise.get_future().get();
}

void TcpServer::handleConnectionClose(const std::shared_ptr<flute::TcpConnection>& conn) {
    auto baseLoop = m_eventLoopGroup->getMasterEventLoop();
    baseLoop->runInLoop(std::bind(&TcpServer::handleConnectionCloseInLoop, this, conn));
}

void TcpServer::handleAcceptConnection(flute::socket_type descriptor) {
    if (m_state != ServerState::STARTED) {
        return;
    }
    EventLoop* ioLoop = m_eventLoopGroup->chooseSlaveEventLoop(m_connections.size());
    auto localAddress = flute::getLocalAddr(descriptor);
    auto remoteAddress = flute::getRemoteAddr(descriptor);
    TcpConnectionPtr conn(new TcpConnection(descriptor, ioLoop, localAddress, remoteAddress));
    assert(m_connections.find(descriptor) == m_connections.end());
    m_connections[descriptor] = conn;
    conn->setConnectionDestroyCallback(std::bind(&TcpServer::handleConnectionDestroy, this, std::placeholders::_1));
    conn->setMessageCallback(m_messageCallback);
    conn->setHighWaterMarkCallback(m_highWaterMarkCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(std::bind(&TcpServer::handleConnectionClose, this, std::placeholders::_1));
    conn->setConnectionEstablishedCallback(m_connectionEstablishedCallback);
    conn->handleConnectionEstablished();
}

void TcpServer::handleConnectionCloseInLoop(const std::shared_ptr<flute::TcpConnection>& conn) {
    m_eventLoopGroup->getMasterEventLoop()->assertInLoopThread();
    if (m_closeCallback) {
        m_closeCallback(conn);
    }
    conn->handleConnectionDestroy();
}

void TcpServer::handleConnectionDestroy(const std::shared_ptr<flute::TcpConnection>& conn) {
    auto baseLoop = m_eventLoopGroup->getMasterEventLoop();
    baseLoop->runInLoop(std::bind(&TcpServer::handleConnectionDestroyInLoop, this, conn));
}

void TcpServer::handleConnectionDestroyInLoop(const std::shared_ptr<flute::TcpConnection>& conn) {
    if (m_connectionDestroyCallback) {
        m_connectionDestroyCallback(conn);
    }
    auto n = m_connections.erase(conn->descriptor());
    assert(n == 1);
    (void)n;
    if (m_connections.empty() && m_state == ServerState::STOPPING) {
        m_state = ServerState::STOPPED;
    }
    LOG_TRACE << "remove connection " << conn->getRemoteAddress().toString() << " live count " << m_connections.size();
    if (m_connections.empty()) {
        m_close_promise.set_value();
    }
}

void TcpServer::closeInLoop() {
    m_state = ServerState::STOPPING;
    m_acceptor->close();
    auto isEmpty = m_connections.empty();
    for (auto& it : m_connections) {
        it.second->shutdown();
    }
    if (isEmpty) {
        m_state = ServerState::STOPPED;
        m_close_promise.set_value();
    }
}

} // namespace flute