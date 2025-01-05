//
// Created by why on 2020/01/05.
//

#include <flute/AsyncIoService.h>
#include <flute/Channel.h>
#include <flute/Connector.h>
#include <flute/Logger.h>
#include <flute/flute_types.h>

#include <algorithm>
#include <cassert>
#include <cerrno>

namespace flute {

const int Connector::MAX_RETRY_DELAY = 30 * 1000;
const int Connector::DEFALUT_RETRY_DELAY = 500;

Connector::Connector(EventLoop* loop, const InetAddress& address)
    : m_retryDelay(DEFALUT_RETRY_DELAY)
    , m_loop(loop)
    , m_channel(nullptr)
    , m_ioContext(nullptr)
    , m_serverAddress(address)
    , m_state(ConnectorState::DISCONNECTED)
    , m_isConnect(false)
    , m_connectCallback() {}

Connector::Connector(EventLoop* loop, InetAddress&& address)
    : m_retryDelay(DEFALUT_RETRY_DELAY)
    , m_loop(loop)
    , m_channel(nullptr)
    , m_ioContext(nullptr)
    , m_serverAddress(std::move(address))
    , m_state(ConnectorState::DISCONNECTED)
    , m_isConnect(false)
    , m_connectCallback() {}

Connector::~Connector() {
    assert(!m_channel);
    if (m_channel) {
        m_channel->disableAll();
        delete m_channel;
        m_channel = nullptr;
    }
    auto asyncIoService = m_loop->getAsyncIoService();
    if (m_ioContext && asyncIoService) {
        asyncIoService->destroyIoContext(m_ioContext);
        m_ioContext = nullptr;
    }
}

void Connector::start() {
    m_isConnect = true;
    m_loop->runInLoop(std::bind(&Connector::startInLoop, shared_from_this()));
}

void Connector::stop() {
    m_isConnect = false;
    m_loop->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::startInLoop() {
    m_loop->assertInLoopThread();
    assert(m_state == ConnectorState::DISCONNECTED);
    if (m_isConnect) {
        connect();
    } else {
        LOG_DEBUG << "do not connect.";
    }
}

void Connector::stopInLoop() {
    m_loop->assertInLoopThread();
    if (m_state == ConnectorState::CONNECTING) {
        m_state = ConnectorState::DISCONNECTED;
        auto asyncIoService = m_loop->getAsyncIoService();
        if (asyncIoService) {
            retry(m_ioContext->socket);
        } else {
            socket_type sockfd = removeAndResetChannel();
            retry(sockfd);
        }
    }
}

void Connector::connect() {
    auto descriptor = flute::createNonblockingSocket(m_serverAddress.family(), SocketType::STREAM_SOCKET);
    auto asyncIoService = m_loop->getAsyncIoService();
    if (asyncIoService) {
        m_ioContext = asyncIoService->createIoContext();
        m_ioContext->socket = descriptor;
        m_ioContext->opCode = SocketOpCode::Connect;
        m_ioContext->userData = m_serverAddress.getSocketAddress();
        m_ioContext->ioCompleteCallback = std::bind(&Connector::handleAsyncConnect, this, std::placeholders::_1,
                                                    std::placeholders::_2, std::placeholders::_3);
        asyncIoService->bindIoService(descriptor);
        asyncIoService->post(m_ioContext);
        return;
    }
    int ret = flute::connect(descriptor, m_serverAddress);
    auto savedErrno = (ret == 0) ? 0 : flute::getSocketError(descriptor);
    switch (savedErrno) {
        case 0:
        case FLUTE_ERROR(EINPROGRESS):
        case FLUTE_ERROR(EINTR):
        case FLUTE_ERROR(EISCONN):
        case FLUTE_ERROR(EWOULDBLOCK):
#if !defined(_WIN32) && EAGAIN != EWOULDBLOCK
        case FLUTE_ERROR(EAGAIN):
#endif
            connecting(descriptor);
            break;
        case FLUTE_ERROR(EADDRINUSE):
        case FLUTE_ERROR(EADDRNOTAVAIL):
        case FLUTE_ERROR(ECONNREFUSED):
        case FLUTE_ERROR(ENETUNREACH):
            retry(descriptor);
            break;

        case FLUTE_ERROR(EACCES):
#ifndef _WIN32
        case FLUTE_ERROR(EPERM):
#endif
        case FLUTE_ERROR(EAFNOSUPPORT):
        case FLUTE_ERROR(EALREADY):
        case FLUTE_ERROR(EBADF):
        case FLUTE_ERROR(EFAULT):
        case FLUTE_ERROR(ENOTSOCK):
            LOG_ERROR << "connect error in Connector::startInLoop " << savedErrno;
            flute::closeSocket(descriptor);
            break;

        default:
            LOG_ERROR << "Unexpected error in Connector::startInLoop " << savedErrno;
            flute::closeSocket(descriptor);
            break;
    }
}

void Connector::restart() {
    m_loop->assertInLoopThread();
    m_state = ConnectorState::DISCONNECTED;
    m_retryDelay = DEFALUT_RETRY_DELAY;
    m_isConnect = true;
    startInLoop();
}

void Connector::connecting(socket_type descriptor) {
    m_state = ConnectorState::CONNECTING;
    assert(!m_channel);
    m_channel = new Channel(descriptor, m_loop, m_loop->getAsyncIoService() != nullptr);
    m_channel->setWriteCallback(std::bind(&Connector::handleWrite, this));
    m_channel->enableWrite();
}

void Connector::handleWrite() {
    if (m_state != ConnectorState::CONNECTING) {
        assert(m_state == ConnectorState::DISCONNECTED);
        return;
    }
    auto descriptor = removeAndResetChannel();
    auto error = flute::getSocketError(descriptor);
    if (error) {
        LOG_WARN << "Connector::handleRead - socket error " << error << ":" << flute::formatErrorString(error);
        retry(descriptor);
    } else if (flute::isSelfConnect(descriptor)) {
        LOG_WARN << "Connector::handleRead - self connection";
        retry(descriptor);
    } else {
        m_state = ConnectorState::CONNECTED;
        if (m_isConnect && m_connectCallback) {
            m_connectCallback(descriptor);
        } else {
            flute::closeSocket(descriptor);
        }
    }
}

void Connector::handleTimeout() {
    if (m_state != ConnectorState::CONNECTING) {
        assert(m_state == ConnectorState::DISCONNECTED);
        return;
    }
    auto descriptor = removeAndResetChannel();
    retry(descriptor);
}

void Connector::resetChannel() {
    if (m_channel) {
        delete m_channel;
    }
    m_channel = nullptr;
}

socket_type Connector::removeAndResetChannel() {
    m_channel->disableAll();
    auto descriptor = m_channel->descriptor();
    m_loop->queueInLoop(std::bind(&Connector::resetChannel, this));
    return descriptor;
}

#ifdef min
#undef min
#endif

void Connector::retry(socket_type descriptor) {
    flute::closeSocket(descriptor);
    m_state = ConnectorState::DISCONNECTED;
    if (m_isConnect) {
        LOG_INFO << "retry connecting to " << m_serverAddress.toString() << " in " << m_retryDelay << " milliseconds.";
        m_loop->schedule(std::bind(&Connector::startInLoop, shared_from_this()), m_retryDelay, 1);
        m_retryDelay = std::min(m_retryDelay << 1, MAX_RETRY_DELAY);
    } else {
        LOG_DEBUG << "do not connect.";
    }
}

void Connector::handleAsyncConnect(AsyncIoCode code, ssize_t bytes, AsyncIoContext* ioContext) {
    m_loop->runInLoop(std::bind(&Connector::handleAsyncConnectInLoop, this, code, bytes, ioContext));
}

void Connector::handleAsyncConnectInLoop(AsyncIoCode code, ssize_t bytes, AsyncIoContext* ioContext) {
    if (m_state == ConnectorState::CONNECTING) {
        if (code == AsyncIoCode::IoCodeSuccess) {
            m_state = ConnectorState::CONNECTED;
            if (m_isConnect && m_connectCallback) {
                m_connectCallback(ioContext->socket);
            }
        }
    } else if (code == AsyncIoCode::IoCodeFailed) {
        retry(ioContext->socket);
    }
}

} // namespace flute
