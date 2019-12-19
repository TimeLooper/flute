/*************************************************************************
 *
 * File Name:  Connector.cpp
 * Repository: https://github.com/TimeLooper/flute
 * Author:     why
 * Date:       2019/12/19 23:46:39
 *
 *************************************************************************/

#include <flute/Connector.h>
#include <flute/Channel.h>
#include <flute/EventLoop.h>
#include <flute/EventLoopGroup.h>
#include <flute/Logger.h>

namespace flute {

const int Connector::MAX_RETRY_DELAY_TIME = 30 * 1000;
const int Connector::DEFAULT_RETRY_DELAY_TIME = 500;

Connector::Connector(EventLoopGroup* loop, const InetAddress& address)
    : m_retryDelay(DEFAULT_RETRY_DELAY_TIME)
    , m_loop(loop->chooseEventLoop())
    , m_remoteAddress(address)
    , m_connect(false)
    , m_state(ConnectorState::DISCONNECTED)
    , m_channel(nullptr) {
}

Connector::Connector(EventLoopGroup* loop, InetAddress&& address)
    : m_loop(loop->chooseEventLoop()), m_remoteAddress(std::move(address)), m_state(ConnectorState::DISCONNECTED), m_channel(nullptr) {
}

Connector::~Connector() {
    assert(!m_channel);
}

void Connector::start() {
    m_connect = true;
    m_loop->runInLoop(std::bind(&Connector::startInLoop, this));
}

void Connector::stop() {
    m_connect = false;
    m_loop->queueInLoop(std::bind(&Connector::stopInLoop, this));
}

void Connector::startInLoop() {
    m_loop->assertInLoopThread();
    assert(m_state == ConnectorState::DISCONNECTED);
    if (m_connect) {
        connect();
    } else {
        LOG_TRACE << "do not connect.";
    }
}

void Connector::stopInLoop() {
    m_loop->assertInLoopThread();
    if (m_state == ConnectorState::CONNECTING) {
        setState(ConnectorState::DISCONNECTED);
        int sockfd = removeAndResetChannel();
        retry(sockfd);
    }
}

void Connector::connect() {
    auto fd = flute::createNonblockingSocket(m_remoteAddress.family());
    auto ret = flute::connect(fd, m_remoteAddress);
    auto error = (ret == 0) ? 0 : errno;
    switch (error) {
        case 0:
        case EINPROGRESS:
        case EINTR:
        case EISCONN:
            connecting(fd);
            break;
        case EAGAIN:
        case EADDRINUSE:
        case EADDRNOTAVAIL:
        case ECONNREFUSED:
        case ENETUNREACH:
            retry(fd);
            break;
        case EACCES:
        case EPERM:
        case EAFNOSUPPORT:
        case EALREADY:
        case EBADF:
        case EFAULT:
        case ENOTSOCK:
            LOG_ERROR << "connect error in Connector::internalConnect " << error << ":" << std::strerror(error);
            flute::closeSocket(fd);
            break;
        default:
            LOG_ERROR << "unexcepted error in Connector::internalConnect " << error << ":" << std::strerror(error);
            flute::closeSocket(fd);
            break;
    }
}

void Connector::restart() {
    m_loop->assertInLoopThread();
    setState(ConnectorState::DISCONNECTED);
    m_retryDelay = DEFAULT_RETRY_DELAY_TIME;
    m_connect = true;
    startInLoop();
}

void Connector::connecting(socket_type fd) {
    setState(ConnectorState::CONNECTING);
    assert(!m_channel);
    m_channel.reset(new Channel(fd, m_loop));
    m_channel->setWriteCallback(std::bind(&Connector::handleWrite, this));
    m_channel->enableWrite();
}

void Connector::retry(socket_type fd) {
    flute::closeSocket(fd);
    setState(ConnectorState::DISCONNECTED);
    if (m_connect) {
        LOG_INFO << "Retry connecting to " << m_remoteAddress.toString() << " in " << m_retryDelay << " milliseconds.";
        m_loop->schedule(std::bind(&Connector::startInLoop, this), m_retryDelay);
        m_retryDelay = std::min(m_retryDelay * 2, MAX_RETRY_DELAY_TIME);
    } else {
        LOG_DEBUG << "do not connect.";
    }
}

void Connector::handleWrite() {
    if (m_state == ConnectorState::CONNECTING) {
        auto descriptor = removeAndResetChannel();
        auto error = flute::getSocketError(descriptor);
        if (error) {
            LOG_WARN << "Connector::handleWrite - SO_ERROR = " << error << " " << std::strerror(error);
            retry(descriptor);
        } else if (flute::isSelfConnect(descriptor)) {
            LOG_WARN << "self connect.";
            retry(descriptor);
        } else {
            setState(ConnectorState::CONNECTED);
            if (m_connect) {
                if (m_connectedCallback) {
                    m_connectedCallback(descriptor);
                }
            } else {
                flute::closeSocket(descriptor);
            }
        }
    } else {
        assert(m_state == ConnectorState::DISCONNECTED);
    }
}

// void Connector::handleError() {
//     LOG_ERROR << "Connector::handleError state = " << m_state;
//     if (m_state == ConnectorState::CONNECTING) {
//         auto descriptor = removeAndResetChannel();
//         auto err = flute::getSocketError(descriptor);
//         LOG_TRACE << "SO_ERROR = " << err << std::strerror(err);
//         retry(descriptor);
//     }
// }

socket_type Connector::removeAndResetChannel() {
    m_channel->disableAll();
    auto descriptor = m_channel->descriptor();
    m_loop->queueInLoop(std::bind(&Connector::resetChannel, this));
    return descriptor;
}

void Connector::resetChannel() {
    m_channel.reset();
}

} // namespace flute