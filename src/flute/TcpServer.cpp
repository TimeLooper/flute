/*************************************************************************
 *
 * File Name:  TcpServer.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/16
 *
 *************************************************************************/

#include <flute/Acceptor.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/TcpConnection.h>
#include <flute/TcpServer.h>

namespace flute {

TcpServer::TcpServer(flute::EventLoopGroup* parent) : TcpServer(parent, parent) {}

TcpServer::TcpServer(flute::EventLoopGroup* parent, flute::EventLoopGroup* child)
    : m_parent(parent)
    , m_child(child)
    , m_state(ServerState::STOPPED)
    , m_acceptor()
    , m_connections()
    , m_serverPromise()
    , m_messageCallback()
    , m_writeCompleteCallback()
    , m_highWaterMarkCallback()
    , m_connectionDestroyCallback()
    , m_closeCallback() {}

TcpServer::~TcpServer() {
    LOG_TRACE << "deconstruct TcpServer " << this;
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
    m_acceptor.reset(new Acceptor(m_parent, address, true));
    m_acceptor->setAcceptCallback(std::bind(&TcpServer::handleAcceptConnection, this, std::placeholders::_1));
    assert(!m_acceptor->listening());
    m_acceptor->listen();
}

void TcpServer::sync() { m_serverPromise.get_future().get(); }

void TcpServer::close() {
    assert(m_state != ServerState::STOPPING);
    if (m_state == ServerState::STOPPING) {
        return;
    }
    m_state = ServerState::STOPPING;
    m_acceptor->close();
    for (auto& it : m_connections) {
        it.second->shutdown();
    }
}

void TcpServer::handleConnectionClose(const std::shared_ptr<flute::TcpConnection>& conn) {
    if (m_closeCallback) {
        m_closeCallback(conn);
    }
    auto baseLoop = m_acceptor->getEventLoop();
    baseLoop->runInLoop(std::bind(&TcpServer::handleConnectionCloseInLoop, this, conn));
}

void TcpServer::handleAcceptConnection(flute::socket_type descriptor) {
    if (m_state != ServerState::STARTED) {
        return;
    }
    EventLoop* ioLoop = m_child->chooseEventLoop(descriptor);
    auto localAddress = flute::getLocalAddr(descriptor);
    auto remoteAddress = flute::getRemoteAddr(descriptor);
    std::shared_ptr<TcpConnection> conn(new TcpConnection(descriptor, ioLoop, localAddress, remoteAddress));
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
    m_acceptor->getEventLoop()->assertInLoopThread();
    conn->handleConnectionDestroy();
}

void TcpServer::handleConnectionDestroy(const std::shared_ptr<flute::TcpConnection>& conn) {
    if (m_connectionDestroyCallback) {
        m_connectionDestroyCallback(conn);
    }
    auto baseLoop = m_acceptor->getEventLoop();
    baseLoop->runInLoop(std::bind(&TcpServer::handleConnectionDestroyInLoop, this, conn));
}

void TcpServer::handleConnectionDestroyInLoop(const std::shared_ptr<flute::TcpConnection>& conn) {
    auto n = m_connections.erase(conn->descriptor());
    assert(n == 1);
    (void)n;
    if (m_connections.empty() && m_state == ServerState::STOPPING) {
        m_state = ServerState::STOPPED;
        m_serverPromise.set_value();
    }
}

} // namespace flute