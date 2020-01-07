//
// Created by why on 2020/01/05.
//

#include <flute/Connector.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopGroup.h>
#include <flute/InetAddress.h>
#include <flute/Logger.h>
#include <flute/TcpClient.h>
#include <flute/TcpConnection.h>

#include <cassert>

namespace flute {

TcpClient::TcpClient(EventLoopGroup* loopGroup, const InetAddress& address)
    : m_loopGroup(loopGroup)
    , m_retry(false)
    , m_isConnect(true)
    , m_mutex()
    , m_connector(new Connector(loopGroup->getMasterEventLoop(), address))
    , m_connectionEstablishedCallback()
    , m_messageCallback()
    , m_writeCompleteCallback() {
    m_connector->setConnectCallback(std::bind(&TcpClient::onConnectSuccess, this, std::placeholders::_1));
}

TcpClient::TcpClient(EventLoopGroup* loopGroup, InetAddress&& address)
    : m_loopGroup(loopGroup)
    , m_retry(false)
    , m_isConnect(true)
    , m_mutex()
    , m_connector(new Connector(loopGroup->getMasterEventLoop(), std::move(address)))
    , m_connectionEstablishedCallback()
    , m_messageCallback()
    , m_writeCompleteCallback() {
    m_connector->setConnectCallback(std::bind(&TcpClient::onConnectSuccess, this, std::placeholders::_1));
}

TcpClient::~TcpClient() {
    TcpConnectionPtr conn;
    bool unique = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        unique = m_connection.unique();
        conn = m_connection;
    }
    if (conn) {
        conn->getEventLoop()->runInLoop(
            [=] { conn->setCloseCallback(std::bind(&TcpConnection::handleConnectionDestroy, std::placeholders::_1)); });
        if (unique) {
            conn->forceClose();
        }
    } else {
        m_connector->stop();
    }
}

void TcpClient::connect() {
    LOG_INFO << "connecting to " << m_connector->getServerAddress().toString();
    m_isConnect = true;
    m_connector->start();
}

void TcpClient::disconnect() {
    m_isConnect = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_connection) {
            m_connection->shutdown();
        }
    }
}

void TcpClient::stop() {
    m_isConnect = false;
    m_connector->stop();
}

void TcpClient::onConnectSuccess(socket_type descriptor) {
    m_loopGroup->getMasterEventLoop()->assertInLoopThread();
    auto remoteAddress = flute::getRemoteAddr(descriptor);
    auto localAddress = flute::getLocalAddr(descriptor);
    TcpConnectionPtr conn(
        new TcpConnection(descriptor, m_loopGroup->chooseSlaveEventLoop(descriptor), localAddress, remoteAddress));
    conn->setConnectionEstablishedCallback(m_connectionEstablishedCallback);
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connection = conn;
    }
    conn->handleConnectionEstablished();
}

void TcpClient::removeConnection(const TcpConnectionPtr& connection) {
    connection->getEventLoop()->assertInLoopThread();
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        assert(connection == m_connection);
        m_connection.reset();
    }
    connection->handleConnectionDestroy();
    if (m_retry && m_isConnect) {
        LOG_INFO << "connection to " << connection->getRemoteAddress().toString();
        m_connector->restart();
    }
}

} // namespace flute
