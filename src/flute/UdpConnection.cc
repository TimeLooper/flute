//
// Created by why on 2020/01/05.
//

#include <flute/UdpConnection.h>
#include <flute/Channel.h>
#include <flute/Logger.h>
#include <flute/EventLoop.h>

#include <cassert>

namespace flute {

UdpConnection::UdpConnection(socket_type descriptor, EventLoop* loop, const InetAddress& localAddress,
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
    , m_connectionEstablishedCallback()
    , m_connectionDestroyCallback()
    , m_inputBuffer()
    , m_outputBuffer() {
    m_socket->setTcpNoDelay(true);
    m_socket->setKeepAlive(true);
    m_channel->setReadCallback(std::bind(&UdpConnection::handleRead, this));
    m_channel->setWriteCallback(std::bind(&UdpConnection::handleWrite, this));
}

UdpConnection::~UdpConnection() { assert(m_state == ConnectionState::DISCONNECTED); }

void UdpConnection::send(const void* buffer, flute::ssize_t length) {
    if (m_state != ConnectionState::CONNECTED) {
        return;
    }
    if (m_loop->isInLoopThread()) {
        sendInLoop(buffer, length);
    } else {
        void (UdpConnection::*func)(const void*, flute::ssize_t) = &UdpConnection::sendInLoop;
        m_loop->runInLoop(std::bind(func, shared_from_this(), buffer, length));
    }
}

void UdpConnection::send(const std::string& message) {
    if (m_state != ConnectionState::CONNECTED) {
        return;
    }
    if (m_loop->isInLoopThread()) {
        sendInLoop(message);
    } else {
        void (UdpConnection::*func)(const std::string&) = &UdpConnection::sendInLoop;
        m_loop->runInLoop(std::bind(func, shared_from_this(), message));
    }
}

void UdpConnection::send(Buffer& buffer) {
    if (m_state != ConnectionState::CONNECTED) {
        return;
    }
    if (m_loop->isInLoopThread()) {
        sendInLoop(buffer);
    } else {
        void (UdpConnection::*func)(Buffer & buffer) = &UdpConnection::sendInLoop;
        m_loop->runInLoop(std::bind(func, shared_from_this(), buffer));
    }
}

void UdpConnection::handleConnectionEstablished() {
    m_loop->runInLoop(std::bind(&UdpConnection::handleConnectionEstablishedInLoop, this));
}

void UdpConnection::handleConnectionDestroy() {
    m_loop->runInLoop(std::bind(&UdpConnection::handleConnectionDestroyInLoop, this));
}

void UdpConnection::startRead() {
    auto ptr = shared_from_this();
    m_loop->runInLoop([ptr] { ptr->m_channel->enableRead(); });
}

void UdpConnection::stopRead() {
    auto ptr = shared_from_this();
    m_loop->runInLoop([ptr] { ptr->m_channel->disableRead(); });
}

void UdpConnection::startWrite() {
    auto ptr = shared_from_this();
    m_loop->runInLoop([ptr] { ptr->m_channel->enableWrite(); });
}

void UdpConnection::stopWrite() {
    auto ptr = shared_from_this();
    m_loop->runInLoop([ptr] { ptr->m_channel->disableWrite(); });
}

void UdpConnection::handleRead() {
    m_loop->assertInLoopThread();
    auto result = m_inputBuffer.readFromSocket(m_channel->descriptor());
    if (result > 0) {
        if (m_messageCallback) {
            m_messageCallback(shared_from_this(), m_inputBuffer);
        }
    } else if (result == 0) {
        handleClose();
    } else {
        handleError();
    }
}

void UdpConnection::handleWrite() {
    m_loop->assertInLoopThread();
    if (m_channel->isWriteable()) {
        auto count = m_outputBuffer.sendToSocket(m_channel->descriptor());
        if (count > 0) {
            m_channel->disableWrite();
            if (m_outputBuffer.readableBytes() <= 0) {
                if (m_state == ConnectionState::DISCONNECTING) {
                    shutdownInLoop();
                }
            }
        } else {
            auto error = m_socket->getSocketError();
            LOG_ERROR << "UdpConnection::handleWrite with error " << error << ":" << formatErrorString(error) << ".";
        }
    } else {
        LOG_ERROR << "UdpConnection descriptor " << m_channel->descriptor() << " is down.";
    }
}

void UdpConnection::handleClose() {
    assert(m_state == ConnectionState::CONNECTED || m_state == ConnectionState::DISCONNECTING);
    m_state = ConnectionState::DISCONNECTING;
    m_channel->disableAll();
    auto conn = shared_from_this();
}

void UdpConnection::handleError() {
    auto error = m_socket->getSocketError();
    LOG_ERROR << "UdpConnection::handleError " << m_channel->descriptor() << " - SO_ERROR = " << error << " "
              << formatErrorString(error);
    if (error == FLUTE_ERROR(ECONNRESET)) {
        handleClose();
    }
}

void UdpConnection::shutdownInLoop() {
    m_loop->assertInLoopThread();
    m_socket->shutdownWrite();
}

void UdpConnection::sendInLoop(const void* buffer, flute::ssize_t length) {
    m_loop->assertInLoopThread();
    if (m_state == ConnectionState::DISCONNECTED) {
        LOG_WARN << "write bytes to a disconnected connection.";
        return;
    }
    bool error = false;
    flute::ssize_t count = 0;
    flute::ssize_t remain = length;
    if (!m_channel->isWriteable() && m_outputBuffer.readableBytes() == 0) {
        iovec vec{};
        vec.iov_base = reinterpret_cast<char*>(const_cast<void*>(buffer));
        vec.iov_len = static_cast<std::size_t>(length);
        // count = flute::writev(m_socket->descriptor(), &vec, 1);
        msghdr message{};
        message.msg_name = const_cast<sockaddr *>(m_remoteAddress.getSocketAddress());
        message.msg_namelen = m_remoteAddress.getSocketLength();
        message.msg_iov = &vec;
        message.msg_iovlen = 1;
        flute::sendmsg(m_socket->descriptor(), &message, 0);
        if (count >= 0) {
            remain = length - count;
            // if (remain <= 0 && m_writeCompleteCallback) {
            //     m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            // }
        } else {
            count = 0;
            auto error_code = m_socket->getSocketError();
            if (error_code != FLUTE_ERROR(EWOULDBLOCK) && error_code != EAGAIN) {
                LOG_ERROR << "UdpConnection::sendInLoop " << error_code << ":" << formatErrorString(error_code);
                if (error_code == EPIPE || error_code == FLUTE_ERROR(ECONNRESET)) {
                    error = true;
                }
            }
        }
    }

    assert(remain <= length);
    if (!error && remain > 0) {
        auto len = m_outputBuffer.readableBytes();
        // if (len + remain >= m_highWaterMark && len < m_highWaterMark && m_highWaterMarkCallback) {
        //     m_loop->queueInLoop(std::bind(m_highWaterMarkCallback, shared_from_this(), len + remain));
        // }
        m_outputBuffer.append(reinterpret_cast<const std::uint8_t*>(buffer) + count, remain);
        if (!m_channel->isWriteable()) {
            m_channel->enableWrite();
        }
    }
}

void UdpConnection::sendInLoop(const std::string& message) {
    sendInLoop(message.c_str(), static_cast<flute::ssize_t>(message.length()));
}

void UdpConnection::sendInLoop(Buffer& buffer) {
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
            // if (remain <= 0 && m_writeCompleteCallback) {
            //     m_loop->queueInLoop(std::bind(m_writeCompleteCallback, shared_from_this()));
            // }
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
        // if (len + remain >= m_highWaterMark && len < m_highWaterMark && m_highWaterMarkCallback) {
        //     m_loop->queueInLoop(std::bind(m_highWaterMarkCallback, shared_from_this(), len + remain));
        // }
        m_outputBuffer.append(buffer);
        if (!m_channel->isWriteable()) {
            m_channel->enableWrite();
        }
    }
}

void UdpConnection::handleConnectionEstablishedInLoop() {
    m_loop->assertInLoopThread();
    assert(m_state == ConnectionState::CONNECTING || m_state == ConnectionState::DISCONNECTED);
    m_state = ConnectionState::CONNECTED;
    m_channel->enableRead();
    if (m_connectionEstablishedCallback) {
        m_connectionEstablishedCallback(shared_from_this());
    }
}

void UdpConnection::handleConnectionDestroyInLoop() {
    m_loop->assertInLoopThread();
    if (m_state == ConnectionState::DISCONNECTING) {
        m_state = ConnectionState::DISCONNECTED;
        m_channel->disableAll();
        m_socket->close();
        if (m_connectionDestroyCallback) {
            m_connectionDestroyCallback(shared_from_this());
        }
    }
}

void UdpConnection::forceCloseInLoop() {
    m_loop->assertInLoopThread();
    if (m_state == ConnectionState::CONNECTED || m_state == ConnectionState::CONNECTING) {
        handleClose();
    }
}

} // namespace flute