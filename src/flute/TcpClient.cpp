/*************************************************************************
 *
 * File Name:  TcpClient.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/19 23:57:24
 *
 *************************************************************************/

#include <flute/Connector.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopGroup.h>
#include <flute/Logger.h>
#include <flute/TcpClient.h>
#include <flute/TcpConnection.h>

#include <cassert>

namespace flute {

static void handleRemoveConnection(const std::shared_ptr<TcpConnection>& conn) { conn->handleConnectionDestroy(); }

static void removeConnector(const std::shared_ptr<Connector>& connector) {
    // connector->
}

TcpClient::TcpClient(EventLoopGroup* loop, const InetAddress& address)
    : m_loop(loop)
    , m_retry(false)
    , m_connect(true)
    , m_isEstablished(false)
    , m_connectPromise()
    , m_closePromise()
    , m_mutex()
    , m_connector(new Connector(loop, address))
    , m_messageCallback()
    , m_writeCompleteCallback() {
    m_connector->setOnConnectedCallback(std::bind(&TcpClient::onConnected, this, std::placeholders::_1));
}

TcpClient::~TcpClient() {
    std::shared_ptr<TcpConnection> conn;
    bool unique = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        unique = m_connection.unique();
        conn = m_connection;
    }
    if (conn) {
        // assert(m_loop == conn->getEventLoop());
        auto cb = std::bind(&handleRemoveConnection, std::placeholders::_1);
        conn->getEventLoop()->runInLoop([=] { conn->setCloseCallback(cb); });
        if (unique) {
            conn->forceClose();
        }
    } else {
        m_connector->stop();
        conn->getEventLoop()->schedule(std::bind(&removeConnector, m_connector), 1000);
    }
}

std::future<void> TcpClient::connect() {
    LOG_INFO << "connecting to " << m_connector->getRemoteAddress().toString();
    m_connect = true;
    m_connector->start();
    return m_connectPromise.get_future();
}

void TcpClient::disconnect() {
    m_connect = false;
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_connection) {
            m_connection->shutdown();
        }
    }
    m_isEstablished = false;
    m_connectPromise.set_value();
    m_closePromise.set_value();
}

void TcpClient::stop() {
    auto temp = m_connect.load();
    m_connect = false;
    m_connector->stop();
    m_isEstablished = false;
    if (temp) {
        m_connectPromise.set_value();
    }
}

void TcpClient::sync() {
    // if (!m_isEstablished) {
    //     return;
    // }
    m_closePromise.get_future().get();
}

void TcpClient::onConnected(socket_type descriptor) {
    auto remoteAddress = flute::getRemoteAddr(descriptor);
    auto localAddress = flute::getLocalAddr(descriptor);
    std::shared_ptr<TcpConnection> conn(
        new TcpConnection(descriptor, m_loop->chooseEventLoop(descriptor), localAddress, remoteAddress));
    conn->setMessageCallback(m_messageCallback);
    conn->setWriteCompleteCallback(m_writeCompleteCallback);
    conn->setConnectionEstablishedCallback(m_connectionEstablishedCallback);
    conn->setCloseCallback(std::bind(&TcpClient::removeConnection, this, std::placeholders::_1));
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_connection = conn;
    }
    conn->handleConnectionEstablished();
    m_isEstablished = true;
    m_connectPromise.set_value();
}

void TcpClient::removeConnection(const std::shared_ptr<TcpConnection>& connection) {
    connection->getEventLoop()->assertInLoopThread();
    // assert(m_loop == connection->getEventLoop());
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        assert(connection == m_connection);
        m_connection.reset();
    }
    // m_loop->runInLoop(std::bind(&TcpConnection::connectionDestroy, connection));
    connection->handleConnectionDestroy();
    if (m_retry && m_connect) {
        LOG_INFO << "connection to " << connection->getRemoteAddress().toString();
        m_connector->restart();
    }
}

} // namespace flute