/*************************************************************************
 *
 * File Name:  Connection.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/05 23:46:29
 *
 *************************************************************************/

#include <flute/Channel.h>
#include <flute/Connection.h>
#include <flute/Logger.h>
#include <flute/socket_ops.h>

#include <cassert>
#include <cerrno>
#include <cstring>

namespace flute {

Connection::Connection(socket_type descriptor, EventLoop* loop, const sockaddr_storage& localAddress,
                       const sockaddr_storage& remoteAddress)
    : m_descriptor(descriptor)
    , m_highWaterMark(0)
    , m_loop(loop)
    , m_state(ConnectionState::DISCONNECTED)
    , m_channel(new Channel(descriptor, loop))
    , m_localAddress(localAddress)
    , m_remoteAddress(remoteAddress)
    , m_messageCallback()
    , m_closeCallback()
    , m_writeCompleteCallback()
    , m_highWaterMarkCallback()
    , m_inputBuffer()
    , m_outputBuffer() {
    setTcpNoDelay(true);
    m_channel->setReadCallback(std::bind(&Connection::handleRead, this));
    m_channel->setWriteCallback(std::bind(&Connection::handleWrite, this));
}

Connection::~Connection() {
    assert(m_state == ConnectionState::DISCONNECTED);
}

void Connection::setMessageCallback(const MessageCallback& cb) {
    m_messageCallback = cb;
}

void Connection::setMessageCallback(MessageCallback&& cb) {
    m_messageCallback = std::move(cb);
}

void Connection::setCloseCallback(const CloseCallback& cb) {
    m_closeCallback = cb;
}

void Connection::setCloseCallback(CloseCallback&& cb) {
    m_closeCallback = std::move(cb);
}

void Connection::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    m_writeCompleteCallback = cb;
}

void Connection::setWriteCompleteCallback(WriteCompleteCallback&& cb) {
    m_writeCompleteCallback = std::move(cb);
}

void Connection::setHighWaterMarkCallback(const HighWaterMarkCallback& cb) {
    m_highWaterMarkCallback = cb;
}

void Connection::setHighWaterMarkCallback(HighWaterMarkCallback&& cb) {
    m_highWaterMarkCallback = std::move(cb);
}

socket_type Connection::descriptor() const {
    return m_descriptor;
}

EventLoop* Connection::getEventLoop() const {
    return m_loop;
}

const sockaddr_storage& Connection::getLocalAddress() const {
    return m_localAddress;
}

const sockaddr_storage& Connection::getRemoteAddress() const {
    return m_remoteAddress;
}

int Connection::shutdown(int flags) const {
    return flute::shutdown(m_descriptor, flags);
}

void Connection::send(const void* buffer, std::int32_t length) const {
    if (m_state != ConnectionState::CONNECTED) {
        return;
    }
    if (m_loop->isInLoopThread()) {
        sendInLoop(buffer, length);
    } else {
        auto func = &Connection::sendInLoop;
    }
}

void Connection::send(const std::string& message) const {
}

void Connection::handleRead() {
    m_loop->assertInLoopThread();
    auto result = m_inputBuffer.readFromSocket(m_descriptor);
    m_channel->disableRead();
    if (result > 0) {
        if (m_messageCallback) {
            m_messageCallback(shared_from_this(), m_inputBuffer);
        }
    } else if (result == 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        handleClose();
        return;
    } else {
        auto error = flute::getSocketError(m_descriptor);
        LOG_ERROR << "TcpConnector::handleRead - SO_ERROR = " << error << " : " << std::strerror(error);
    }
    m_channel->enableWrite();
}

void Connection::handleWrite() {
    m_loop->assertInLoopThread();
    auto result = m_outputBuffer.sendToSocket(m_descriptor);
    m_channel->disableWrite();
    if (result > 0) {
        auto remain = m_outputBuffer.readableBytes();
        if (remain <= 0) {
            if (m_writeCompleteCallback) {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            }
            if (m_state == ConnectionState::DISCONNECTING) {
                shutdownInLoop();
                return;
            }
        } else {
            if (remain >= m_highWaterMark) {
                if (m_highWaterMarkCallback) {
                    m_highWaterMarkCallback(shared_from_this(), remain);
                }
            }
        }
    } else {
        LOG_ERROR << "Connection " << m_descriptor << " write with error " << errno << ":" << std::strerror(errno);
    }
    m_channel->enableRead();
}

void Connection::handleClose() {
    assert(m_state == ConnectionState::CONNECTED || m_state == ConnectionState::DISCONNECTING);
    m_state = ConnectionState::DISCONNECTED;
    m_channel->disableAll();
    auto conn = shared_from_this();
    if (m_closeCallback) {
        m_closeCallback(conn);
    }
}

void Connection::shutdownInLoop() {
    m_loop->assertInLoopThread();
    if (!(m_channel->events() & FileEvent::WRITE)) {
        shutdown(SHUT_WR);
    }
}

void Connection::setTcpNoDelay(bool on) const {
    int option = on ? 1 : 0;
    flute::setsockopt(m_descriptor, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char*>(&option), sizeof(option));
}

void Connection::sendInLoop(const void* buffer, std::int32_t length) const {
}

void Connection::sendInLoop(const std::string& message) const {
}

} // namespace flute
