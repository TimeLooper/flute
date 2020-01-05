//
// Created by why on 2020/01/05.
//

#include <flute/Connector.h>
#include <flute/Channel.h>
#include <flute/Logger.h>

#include <cassert>
#include <cerrno>

namespace flute {

const int Connector::MAX_RETRY_DELAY = 30 * 1000;
const int Connector::DEFALUT_RETRY_DELAY = 500;

Connector::Connector(EventLoop* loop, const InetAddress& address)
    : m_retryDelay(DEFALUT_RETRY_DELAY)
    , m_loop(loop)
    , m_channel(nullptr)
    , m_serverAddress(address)
    , m_state(DISCONNECTED)
    , m_isConnect(false)
    , m_connectCallback() {

}

Connector::~Connector() {
    assert(!m_channel);
    if (m_channel) {
        m_channel->disableAll();
        delete m_channel;
        m_channel = nullptr;
    }
}

void Connector::start() {
    m_isConnect = true;
    m_loop->runInLoop(std::bind(&Connector::startInLoop, shared_from_this()));
}

void Connector::startInLoop() {
    m_loop->assertInLoopThread();
    assert(m_state == DISCONNECTED);
    if (m_isConnect) {
        connect();
    } else {
        LOG_DEBUG << "do not connect.";
    }
}

void Connector::connect() {
    auto descriptor = flute::createNonblockingSocket(m_serverAddress.family());
    int ret = flute::connect(descriptor, m_serverAddress);
    auto savedErrno = (ret == 0) ? 0 : flute::getSocketError(descriptor);
    switch (savedErrno) {
    case 0:
    case FLUTE_ERROR(EINPROGRESS):
    case FLUTE_ERROR(EINTR):
    case FLUTE_ERROR(EISCONN):
        connecting(descriptor);
        break;
    case FLUTE_ERROR(EAGAIN):
    case FLUTE_ERROR(EADDRINUSE):
    case FLUTE_ERROR(EADDRNOTAVAIL):
    case FLUTE_ERROR(ECONNREFUSED):
    case FLUTE_ERROR(ENETUNREACH):
        retry(descriptor);
        break;

    case FLUTE_ERROR(EACCES):
    case FLUTE_ERROR(EPERM):
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

void Connector::connecting(socket_type descriptor) {
    m_state = CONNECTING;
    assert(!m_channel);
    m_channel = new Channel(descriptor, m_loop);
    m_channel->setWriteCallback(std::bind(&Connector::handleRead, this));
    m_channel->setReadCallback(std::bind(&Connector::handleRead, this));
    m_channel->enableWrite();
}

void Connector::handleRead() {
    if (m_state != CONNECTING) {
        assert(m_state == DISCONNECTED);
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
        m_state = CONNECTED;
        if (m_isConnect && m_connectCallback) {
            m_connectCallback(descriptor);
        } else {
            flute::closeSocket(descriptor);
        }
    }
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

void Connector::retry(socket_type descriptor) {
    flute::closeSocket(descriptor);
    m_state = DISCONNECTED;
    if (m_isConnect) {
        LOG_INFO << "retry connecting to " << m_serverAddress.toString() << " in " << m_retryDelay << " milliseconds.";
        m_loop->schedule(std::bind(&Connector::startInLoop, shared_from_this()), m_retryDelay, 1);
        m_retryDelay = std::min(m_retryDelay << 1, MAX_RETRY_DELAY);
    } else {
        LOG_DEBUG << "do not connect.";
    }
}

} // namespace flute
