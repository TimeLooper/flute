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

void Connection::send(const void* buffer, flute::ssize_t length) {
    if (m_state != ConnectionState::CONNECTED) {
        return;
    }
    if (m_loop->isInLoopThread()) {
        sendInLoop(buffer, length);
    } else {
        void (Connection::*func)(const void*, flute::ssize_t) = &Connection::sendInLoop;
        m_loop->runInLoop(std::bind(func, this, buffer, length));
    }
}

void Connection::send(const std::string& message) {
    if (m_state == ConnectionState::CONNECTED) {
        return;
    }
    if (m_loop->isInLoopThread()) {
        sendInLoop(message);
    } else {
        void (Connection::*func)(const std::string&) = &Connection::sendInLoop;
        m_loop->runInLoop(std::bind(func, this, message));
    }
}

void Connection::send(Buffer& buffer) {
    if (m_state != ConnectionState::CONNECTED) {
        return;
    }
    if (m_loop->isInLoopThread()) {
        sendInLoop(buffer);
    } else {
        void (Connection::*func)(Buffer & buffer) = &Connection::sendInLoop;
    }
}

void Connection::handleRead() {
    m_loop->assertInLoopThread();
    auto result = m_inputBuffer.readFromSocket(m_descriptor);
    if (result > 0) {
        if (m_messageCallback) {
            m_messageCallback(shared_from_this(), m_inputBuffer);
        }
    } else if (result == 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        handleClose();
    } else {
        handleError();
    }
}

void Connection::handleWrite() {
    m_loop->assertInLoopThread();
    if (m_channel->isWriteable()) {
        auto count = m_outputBuffer.sendToSocket(m_descriptor);
        if (count > 0) {
            m_channel->disableWrite();
            if (m_outputBuffer.readableBytes() <= 0) {
                if (m_writeCompleteCallback) {
                    m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
                }
                if (m_state == ConnectionState::DISCONNECTING) {
                    shutdownInLoop();
                }
            }
        } else {
            LOG_ERROR << "Connection::handleWrite with error " << errno << ":" << std::strerror(errno) << ".";
        }
    } else {
        LOG_ERROR << "Connection descriptor " << m_descriptor << " is down.";
    }
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

void Connection::handleError() {
    auto error = flute::getSocketError(m_descriptor);
    LOG_ERROR << "Connection::handleError " << m_descriptor << " - SO_ERROR = " << error << " " << std::strerror(error);
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

void Connection::sendInLoop(const void* buffer, flute::ssize_t length) {
    m_loop->assertInLoopThread();
    if (m_state == ConnectionState::DISCONNECTED) {
        LOG_WARN << "write bytes to a disconnected connection.";
        return;
    }
    bool error = false;
    flute::ssize_t count = 0;
    flute::ssize_t remain = length;
    if (!m_channel->isWriteable() && m_outputBuffer.readableBytes() == 0) {
        // try to write direct
        iovec vec = {const_cast<void*>(buffer), static_cast<std::size_t>(length)};
        flute::writev(m_descriptor, &vec, 1);
        if (count >= 0) {
            remain = length - count;
            if (remain <= 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            }
        }
    } else {
        count = 0;
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            LOG_ERROR << "Connection::sendInLoop";
            if (errno == EPIPE || errno == ECONNRESET) {
                error = true;
            }
        }
    }

    assert(remain <= length);
    if (!error && remain > 0) {
        auto len = m_outputBuffer.readableBytes();
        if (len + remain >= m_highWaterMark && len < m_highWaterMark && m_highWaterMarkCallback) {
            m_loop->queueInLoop(std::bind(m_highWaterMarkCallback, shared_from_this(), len + remain));
        }
        m_outputBuffer.append(reinterpret_cast<const std::uint8_t*>(buffer) + count, remain);
        if (!m_channel->isWriteable()) {
            m_channel->enableWrite();
        }
    }
}

void Connection::sendInLoop(const std::string& message) {
    sendInLoop(message.c_str(), message.length());
}

void Connection::sendInLoop(Buffer& buffer) {
    m_loop->assertInLoopThread();
    auto length = buffer.readableBytes();
    if (m_state == ConnectionState::DISCONNECTED) {
        LOG_WARN << "write bytes to a disconnected connection.";
        return;
    }
    bool error = false;
    flute::ssize_t count = 0;
    flute::ssize_t remain = length;
    if (!m_channel->isWriteable() && m_outputBuffer.readableBytes() == 0) {
        count = buffer.sendToSocket(m_descriptor);
        if (count >= 0) {
            remain = length - count;
            if (remain <= 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            }
        }
    } else {
        count = 0;
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            LOG_ERROR << "Connection::sendInLoop";
            if (errno == EPIPE || errno == ECONNRESET) {
                error = true;
            }
        }
    }

    assert(remain <= length);
    if (!error && remain > 0) {
        auto len = m_outputBuffer.readableBytes();
        if (len + remain >= m_highWaterMark && len < m_highWaterMark && m_highWaterMarkCallback) {
            m_loop->queueInLoop(std::bind(m_highWaterMarkCallback, shared_from_this(), len + remain));
        }
        m_outputBuffer.append(buffer);
        if (!m_channel->isWriteable()) {
            m_channel->enableWrite();
        }
    }
}

} // namespace flute
