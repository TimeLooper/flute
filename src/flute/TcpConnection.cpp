/*************************************************************************
 *
 * File Name:  TcpConnection.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/05
 *
 *************************************************************************/

#include <flute/Channel.h>
#include <flute/Logger.h>
#include <flute/TcpConnection.h>
#include <flute/socket_ops.h>

#include <cassert>
#include <cerrno>
#include <cstring>

namespace flute {

TcpConnection::TcpConnection(socket_type descriptor, EventLoop* loop, const InetAddress& localAddress,
                             const InetAddress& remoteAddress)
    : m_highWaterMark(0)
    , m_loop(loop)
    , m_state(ConnectionState::DISCONNECTED)
    , m_channel(new Channel(descriptor, loop))
    , m_socket(new Socket(descriptor))
    , m_localAddress(localAddress)
    , m_remoteAddress(remoteAddress)
    , m_messageCallback()
    , m_closeCallback()
    , m_writeCompleteCallback()
    , m_highWaterMarkCallback()
    , m_connectionEstablishedCallback()
    , m_connectionDestroyCallback()
    , m_inputBuffer()
    , m_outputBuffer() {
    m_socket->setTcpNoDelay(true);
    m_channel->setReadCallback(std::bind(&TcpConnection::handleRead, this));
    m_channel->setWriteCallback(std::bind(&TcpConnection::handleWrite, this));
}

TcpConnection::~TcpConnection() { assert(m_state == ConnectionState::DISCONNECTED); }

void TcpConnection::shutdown() {
    if (m_state == ConnectionState::CONNECTED) {
        m_state = ConnectionState::DISCONNECTING;
        m_loop->runInLoop(std::bind(&TcpConnection::shutdownInLoop, this));
    }
}

void TcpConnection::send(const void* buffer, flute::ssize_t length) {
    if (m_state != ConnectionState::CONNECTED) {
        return;
    }
    if (m_loop->isInLoopThread()) {
        sendInLoop(buffer, length);
    } else {
        void (TcpConnection::*func)(const void*, flute::ssize_t) = &TcpConnection::sendInLoop;
        m_loop->runInLoop(std::bind(func, this, buffer, length));
    }
}

void TcpConnection::send(const std::string& message) {
    if (m_state == ConnectionState::CONNECTED) {
        return;
    }
    if (m_loop->isInLoopThread()) {
        sendInLoop(message);
    } else {
        void (TcpConnection::*func)(const std::string&) = &TcpConnection::sendInLoop;
        m_loop->runInLoop(std::bind(func, this, message));
    }
}

void TcpConnection::send(Buffer& buffer) {
    if (m_state != ConnectionState::CONNECTED) {
        return;
    }
    if (m_loop->isInLoopThread()) {
        sendInLoop(buffer);
    } else {
        void (TcpConnection::*func)(Buffer & buffer) = &TcpConnection::sendInLoop;
        m_loop->runInLoop(std::bind(func, this, buffer));
    }
}

void TcpConnection::handleConnectionEstablished() {
    m_loop->runInLoop(std::bind(&TcpConnection::handleConnectionEstablishedInLoop, this));
}

void TcpConnection::handleConnectionDestroy() {
    m_loop->runInLoop(std::bind(&TcpConnection::handleConnectionDestroyInLoop, this));
}

void TcpConnection::startRead() {
    m_loop->runInLoop([this] {
        if (!this->m_channel->isReadable()) {
            this->m_channel->enableRead();
        }
    });
}

void TcpConnection::stopRead() {
    m_loop->runInLoop([this] {
        if (this->m_channel->isReadable()) {
            this->m_channel->disableRead();
        }
    });
}

void TcpConnection::startWrite() {
    m_loop->runInLoop([this] {
        if (!this->m_channel->isWriteable()) {
            this->m_channel->enableWrite();
        }
    });
}

void TcpConnection::stopWrite() {
    m_loop->runInLoop([this] {
        if (this->m_channel->isWriteable()) {
            this->m_channel->disableWrite();
        }
    });
}

void TcpConnection::handleRead() {
    m_loop->assertInLoopThread();
    auto result = m_inputBuffer.readFromSocket(m_channel->descriptor());
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

void TcpConnection::handleWrite() {
    m_loop->assertInLoopThread();
    if (m_channel->isWriteable()) {
        auto count = m_outputBuffer.sendToSocket(m_channel->descriptor());
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
            LOG_ERROR << "TcpConnection::handleWrite with error " << errno << ":" << std::strerror(errno) << ".";
        }
    } else {
        LOG_ERROR << "TcpConnection descriptor " << m_channel->descriptor() << " is down.";
    }
}

void TcpConnection::handleClose() {
    assert(m_state == ConnectionState::CONNECTED || m_state == ConnectionState::DISCONNECTING);
    m_state = ConnectionState::DISCONNECTED;
    m_channel->disableAll();
    auto conn = shared_from_this();
    if (m_closeCallback) {
        m_closeCallback(conn);
    }
}

void TcpConnection::handleError() {
    auto error = m_socket->getSocketError();
    LOG_ERROR << "TcpConnection::handleError " << m_channel->descriptor() << " - SO_ERROR = " << error << " "
              << std::strerror(error);
}

void TcpConnection::shutdownInLoop() {
    m_loop->assertInLoopThread();
    m_socket->shutdownWrite();
}

void TcpConnection::sendInLoop(const void* buffer, flute::ssize_t length) {
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
        flute::writev(m_socket->descriptor(), &vec, 1);
        if (count >= 0) {
            remain = length - count;
            if (remain <= 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            }
        }
    } else {
        count = 0;
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            LOG_ERROR << "TcpConnection::sendInLoop";
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

void TcpConnection::sendInLoop(const std::string& message) { sendInLoop(message.c_str(), message.length()); }

void TcpConnection::sendInLoop(Buffer& buffer) {
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
        count = buffer.sendToSocket(m_channel->descriptor());
        if (count >= 0) {
            remain = length - count;
            if (remain <= 0 && m_writeCompleteCallback) {
                m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            }
        }
    } else {
        count = 0;
        if (errno != EWOULDBLOCK && errno != EAGAIN) {
            LOG_ERROR << "TcpConnection::sendInLoop";
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

void TcpConnection::handleConnectionEstablishedInLoop() {
    m_loop->assertInLoopThread();
    assert(m_state == ConnectionState::CONNECTING || m_state == ConnectionState::DISCONNECTED);
    m_state = ConnectionState::CONNECTED;
    m_channel->enableRead();
    LOG_DEBUG << "connection established.";
    if (m_connectionEstablishedCallback) {
        m_connectionEstablishedCallback(shared_from_this());
    }
}

void TcpConnection::handleConnectionDestroyInLoop() {
    m_loop->assertInLoopThread();
    if (m_state == ConnectionState::DISCONNECTING) {
        m_state = ConnectionState::DISCONNECTED;
        m_channel->disableAll();
        if (m_connectionDestroyCallback) {
            m_connectionDestroyCallback(shared_from_this());
        }
    }
}

} // namespace flute
